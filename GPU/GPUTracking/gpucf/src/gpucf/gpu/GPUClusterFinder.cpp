#include "GPUClusterFinder.h"

#include <gpucf/ClEnv.h>
#include <gpucf/common/DigitDivider.h>
#include <gpucf/common/log.h>
#include <gpucf/common/RowInfo.h>

#include <shared/tpc.h>

#include <functional>


using namespace gpucf;

using nonstd::optional;


const GPUClusterFinder::Config GPUClusterFinder::defaultConfig;


GPUClusterFinder::Worker::Worker(
        cl::Context context,
        cl::Device  device,
        cl::Program program,
        DeviceMemory mem,
        Worker *prev)
    : mem(mem)
{

    log::Debug() << "create clustering queue.";
    clustering = cl::CommandQueue(
            context, 
            device, 
            CL_QUEUE_PROFILING_ENABLE);

    log::Debug() << "create cleanup queue.";
    cleanup    = cl::CommandQueue(
            context, 
            device, 
            CL_QUEUE_PROFILING_ENABLE);
    log::Debug() << "finished creating cleanup queue.";

    fillChargeMap    = cl::Kernel(program, "fillChargeMap");
    resetChargeMap   = cl::Kernel(program, "resetChargeMap");
    findPeaks        = cl::Kernel(program, "findPeaks");
    computeClusters  = cl::Kernel(program, "computeClusters");

    if (prev != nullptr)
    {
        this->prev = Backwards();
        prev->next = Forwards(); 

        Backwards &slot = *this->prev;
        Forwards  &plug = *prev->next;

        slot.digitsToDevice = plug.digitsToDevice.get_future();
        slot.fillingChargeMap = plug.fillingChargeMap.get_future();
        slot.clusters = plug.clusters.get_future();
        plug.computeClusters = slot.computeClusters.get_future();
    }
}

template<class DigitT>
void GPUClusterFinder::Worker::run(
        const Fragment &range,
        nonstd::span<const DigitT> digits,
        nonstd::span<Cluster> /* cluster */)
{
    if (prev)
    {
        std::vector<cl::Event> ev = {prev->digitsToDevice.get()};
        clustering.enqueueBarrierWithWaitList(&ev);
    }

    /*************************************************************************
     * Copy digits to device
     ************************************************************************/

    const size_t sizeofDigit = sizeof(DigitT);

    nonstd::span<const DigitT> toCopy = digits.subspan(
            range.start + range.backlog, 
            range.items + range.future);

    log::Debug() << "Sending " << toCopy.size() * sizeofDigit << " bytes to device";

    clustering.enqueueWriteBuffer(
            mem.digits,
            CL_FALSE,
            (range.start + range.backlog) * sizeofDigit,
            toCopy.size() * sizeofDigit,
            toCopy.data(),
            nullptr,
            digitsToDevice.get());

    if (next)
    {
        next->digitsToDevice.set_value(*digitsToDevice.get());
    }


    /*************************************************************************
     * Write digits to chargeMap
     ************************************************************************/

    cl::NDRange local(64);

    DBG(range.start);
    DBG(toCopy.size());

    fillChargeMap.setArg(0, mem.digits);
    fillChargeMap.setArg(1, mem.chargeMap);
    clustering.enqueueNDRangeKernel(
            fillChargeMap,
            cl::NDRange(range.start + range.backlog),
            cl::NDRange(toCopy.size()),
            local,
            nullptr,
            fillingChargeMap.get());

    if (next)
    {
        next->fillingChargeMap.set_value(*fillingChargeMap.get());
    }

    if (prev)
    {
        std::vector<cl::Event> ev = {prev->fillingChargeMap.get()};
        clustering.enqueueBarrierWithWaitList(&ev);
    }


    /*************************************************************************
     * Look for peaks
     ************************************************************************/

    DBG(range.start) 
    DBG(range.backlog + range.items);

    findPeaks.setArg(0, mem.chargeMap);
    findPeaks.setArg(1, mem.digits);
    findPeaks.setArg(2, mem.isPeak);
    clustering.enqueueNDRangeKernel(
            findPeaks,
            cl::NDRange(range.start),
            cl::NDRange(range.backlog + range.items),
            local,
            nullptr,
            findingPeaks.get());
}

template<class DigitT>
void GPUClusterFinder::Worker::dispatch(
        const Fragment &fragment,
        nonstd::span<const DigitT> digits,
        nonstd::span<Cluster> clusters)
{
    myThread = std::thread(
            std::bind(
                &GPUClusterFinder::Worker::run<DigitT>, 
                this, 
                fragment, 
                digits, 
                clusters));
}

void GPUClusterFinder::Worker::join()
{
    myThread.join();
    clustering.finish();
}

size_t GPUClusterFinder::computeAndReadClusters()
{
    Worker &worker = workers.front();


    /*************************************************************************
     * Compact peaks
     ************************************************************************/

    size_t clusterNum = streamCompaction.enqueue(
            worker.clustering,
            mem.digits,
            mem.peaks,
            mem.isPeak);

    /* log::Debug() << "Found " << clusterNum << " peaks"; */


    /*************************************************************************
     * Compute cluster
     ************************************************************************/

    cl::NDRange local(64);

    if (clusterNum > 0)
    {
        worker.computeClusters.setArg(0, mem.chargeMap);
        worker.computeClusters.setArg(1, mem.peaks);
        worker.computeClusters.setArg(2, mem.globalToLocalRow);
        worker.computeClusters.setArg(3, mem.globalRowToCru);
        worker.computeClusters.setArg(4, mem.cluster);
        worker.clustering.enqueueNDRangeKernel(
                worker.computeClusters,
                cl::NullRange,
                cl::NDRange(clusterNum),
                local,
                nullptr,
                worker.computingClusters.get());
    }


    /*************************************************************************
     * Reset charge map
     ************************************************************************/

    worker.resetChargeMap.setArg(0, mem.digits);
    worker.resetChargeMap.setArg(1, mem.chargeMap);
    worker.clustering.enqueueNDRangeKernel(
            worker.resetChargeMap,
            cl::NullRange,
            digits.size(),
            local,
            nullptr,
            worker.zeroChargeMap.get());
    

    /*************************************************************************
     * Copy cluster to host
     ************************************************************************/

    ASSERT(clusters.size() == size_t(digits.size()));

    log::Info() << "Copy results back...";

    if (clusterNum > 0)
    {
        worker.clustering.enqueueReadBuffer(
                mem.cluster, 
                CL_TRUE, 
                0, 
                clusterNum * sizeof(Cluster), 
                clusters.data(),
                nullptr,
                worker.clustersToHost.get());
    }

    worker.clustering.finish();

    return clusterNum;
}

/* Lane GPUClusterFinder::Worker::finish() */
/* { */
/*     clustering.finish(); */
/*     cleanup.finish(); */

/*     Lane measurements = */
/*     { */
/*         {"digitsToDevice", digitsToDevice}, */
/*         {"fillChargeMap", fillingChargeMap}, */
/*         {"findPeaks", findingPeaks}, */
/*         parent.streamCompaction.asStep("compactPeaks"), */
/*         {"computeClusters", computingClusters}, */
/*         {"resetChargeMap", zeroChargeMap}, */
/*         {"clustersToHost", clustersToHost}, */
/*     }; */

/*     return measurements; */
/* } */

std::vector<Digit> GPUClusterFinder::getPeaks() const
{
    return peaks;
}

void GPUClusterFinder::setup(Config conf, ClEnv &env, nonstd::span<const Digit> digits)
{
    ASSERT(conf.chunks > 0);

    this->config = conf;
    this->digits = digits;

    if (config.usePackedDigits)
    {
        fillPackedDigits(); 
    }

    addDefines(env);

    streamCompaction.setup(env, digits.size());

    context = env.getContext(); 
    device  = env.getDevice();

    

    
    /*************************************************************************
     * Allocate clusters output
     ************************************************************************/

    clusters.resize(digits.size());


    /*************************************************************************
     * Create Buffer
     ************************************************************************/

    size_t digitsBufSize = 
        ((config.usePackedDigits) ? sizeof(PackedDigit) : sizeof(Digit)) 
        * digits.size();
    mem.digits = cl::Buffer(
            context,
            CL_MEM_READ_WRITE,
            digitsBufSize);

    mem.peaks = cl::Buffer(
            context,
            CL_MEM_READ_WRITE,
            digitsBufSize);

    std::vector<int> globalToLocalRow = RowInfo::instance().globalToLocalMap;
    const size_t numOfRows = globalToLocalRow.size();
    size_t globalToLocalRowBufSize = sizeof(cl_int) * numOfRows;
    mem.globalToLocalRow = cl::Buffer(
            context,
            CL_MEM_READ_ONLY,
            globalToLocalRowBufSize);

    std::vector<int> globalRowToCru = RowInfo::instance().globalRowToCruMap;
    size_t globalRowToCruBufSize = sizeof(cl_int) * numOfRows;
    mem.globalRowToCru = cl::Buffer(
            context,
            CL_MEM_READ_ONLY,
            globalRowToCruBufSize);

    size_t isPeakBufSize = digits.size() * sizeof(cl_int);
    mem.isPeak = cl::Buffer(
            context, 
            CL_MEM_READ_WRITE, 
            isPeakBufSize);

    size_t clusterBufSize = digits.size() * sizeof(Cluster);
    mem.cluster = cl::Buffer(
            context,
            CL_MEM_WRITE_ONLY,
            clusterBufSize);

    log::Info() << "Found " << numOfRows << " rows";

    size_t chargeMapSize  = 
        numOfRows * TPC_PADS_PER_ROW_PADDED 
        * TPC_MAX_TIME_PADDED 
        * sizeof(cl_float);
    mem.chargeMap = cl::Buffer(context, CL_MEM_READ_WRITE, chargeMapSize);


    /************************************************************************
     * Create worker
     ***********************************************************************/

    workers.clear();

    cl::Program cfprg = env.buildFromSrc("clusterFinder.cl");
    for (size_t i = 0; i < config.chunks; i++)
    {
        workers.emplace_back(
                context,
                device,
                cfprg,
                mem,
                (i == 0) ? nullptr : &workers.back());
    }

    
    /*************************************************************************
     * Init constant data
     ************************************************************************/

    log::Debug() << "creating init queue.";
    cl::CommandQueue initQueue(context, device, CL_QUEUE_PROFILING_ENABLE);
    log::Debug() << "finished creating init queue.";

    ASSERT(globalToLocalRowBufSize > 0);
    initQueue.enqueueWriteBuffer(
            mem.globalToLocalRow, 
            CL_FALSE, 
            0, 
            globalToLocalRowBufSize, 
            globalToLocalRow.data());

    ASSERT(globalRowToCruBufSize > 0);
    initQueue.enqueueWriteBuffer(
            mem.globalRowToCru, 
            CL_FALSE, 
            0, 
            globalRowToCruBufSize, 
            globalRowToCru.data());

    ASSERT(chargeMapSize > 0);
    initQueue.enqueueFillBuffer(
            mem.chargeMap, 
            0.0f, 
            0, 
            chargeMapSize);

    initQueue.finish();
}

GPUClusterFinder::Result GPUClusterFinder::run()
{
    static_assert(sizeof(cl_int) == sizeof(int));

    log::Info() << "Looking for clusters...";


    /*************************************************************************
     * Create fragments and run workers
     ************************************************************************/

    DigitDivider divider(digits, config.chunks);

    for (Worker &worker : workers)
    {
        optional<Fragment> fragment = divider.nextChunk(PADDING);

        ASSERT(fragment.has_value());

        log::Debug() << "got new fragment: {start: " << fragment->start
                     << ", backlog: " << fragment->backlog
                     << ", items: " << fragment->items
                     << ", future: " << fragment->future << "}";

        if (config.usePackedDigits)
        {
            worker.dispatch<PackedDigit>(
                    *fragment, 
                    nonstd::span<const PackedDigit>(packedDigits),
                    clusters);
        }
        else
        {
            worker.dispatch(*fragment, digits, clusters);
        }

    }

    for (Worker &worker : workers)
    {
        worker.join();
    }


    /*************************************************************************
     * Compute clusters
     ************************************************************************/

    size_t clusterNum = computeAndReadClusters();

    printClusters(clusters, 10);

    std::vector<Cluster> result(clusterNum);

    memcpy(result.data(), clusters.data(), clusterNum * sizeof(Cluster));

    log::Info() << "Found " << result.size() << " clusters.";

    std::vector<Lane> lanes;
    for (const Worker &worker : workers)
    {
        lanes.push_back(toLane(worker));    
    }

    return Result{result, {0,0, lanes}};
}


void GPUClusterFinder::printClusters(
        const std::vector<Cluster> &clusters,
        size_t maxClusters)
{
    log::Debug() << "Printing found clusters";
    for (size_t i = 0; i < clusters.size(); i++)
    {
        log::Debug() << clusters[i];
        maxClusters--;
        if (maxClusters == 0)
        {
            break;
        }
    }
}

std::vector<Cluster> GPUClusterFinder::filterCluster(
        const std::vector<int> &isCenter,
        const std::vector<Cluster> &clusters)
{
    std::vector<Cluster> actualClusters; 

    for (size_t i = 0; i < clusters.size(); i++)
    {
        if (isCenter[i])
        {
            actualClusters.push_back(clusters[i]);
        }
    }

    return actualClusters;
}

std::vector<Digit> GPUClusterFinder::compactDigits(
        const std::vector<int> &isCenter,
        const std::vector<Digit> &digits)
{
    std::vector<Digit> peaks;    

    for (size_t i = 0; i < digits.size(); i++)
    {
        if (isCenter[i])
        {
            peaks.push_back(digits[i]);
        }
    }

    return peaks;
}

void GPUClusterFinder::fillPackedDigits()
{
    for (const Digit &digit : digits)
    {
        packedDigits.push_back(digit.toPacked());
    }
}

void GPUClusterFinder::addDefines(ClEnv &env)
{
    if (config.usePackedDigits)
    {
        env.addDefine("USE_PACKED_DIGIT");
    }
}


Lane GPUClusterFinder::toLane(const Worker &p)
{
    bool first = (p.prev == nonstd::nullopt);

    return {
        {"digitsToDevice", p.digitsToDevice},
        {"fillChargeMap", p.fillingChargeMap},
        {"findPeaks", p.findingPeaks},
        (first) ? streamCompaction.asStep("compactPeaks") 
                : Step("compactPeaks", 0, 0),
        (first) ? Step("computeCluster", p.computingClusters)
                : Step("computeCluster", 0, 0),
        (first) ? Step("resetChargeMap", p.zeroChargeMap)
                : Step("resetChargeMap", 0, 0),
        (first) ? Step("clusterToHost", p.clustersToHost)
                : Step("clusterToHost", 0, 0),
    };
}

// vim: set ts=4 sw=4 sts=4 expandtab:
