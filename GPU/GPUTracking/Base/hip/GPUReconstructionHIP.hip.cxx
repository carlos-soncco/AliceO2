// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// \file GPUReconstructionHIP.hip.cxx
/// \author David Rohr

#define GPUCA_GPUTYPE_VEGA
#define GPUCA_UNROLL(CUDA, HIP) GPUCA_M_UNROLL_##HIP

#include <hip/hip_runtime.h>
#ifdef __CUDACC__
#define hipExtLaunchKernelGGL(...)
#else
#include <hip/hip_ext.h>
#endif

#include "GPUReconstructionHIP.h"
#include "GPUReconstructionHIPInternals.h"
#include "GPUReconstructionIncludes.h"

using namespace GPUCA_NAMESPACE::gpu;

constexpr size_t gGPUConstantMemBufferSize = (sizeof(GPUConstantMem) + sizeof(uint4) - 1);
#ifndef GPUCA_HIP_NO_CONSTANT_MEMORY
__constant__ uint4 gGPUConstantMemBuffer[gGPUConstantMemBufferSize / sizeof(uint4)];
__global__ void gGPUConstantMemBuffer_dummy(uint4* p) { p[0] = gGPUConstantMemBuffer[0]; }
#define GPUCA_CONSMEM_PTR
#define GPUCA_CONSMEM_CALL
#define GPUCA_CONSMEM (GPUConstantMem&)gGPUConstantMemBuffer
#else
#define GPUCA_CONSMEM_PTR const uint4 *gGPUConstantMemBuffer,
#define GPUCA_CONSMEM_CALL (const uint4*)me->mDeviceConstantMem,
#define GPUCA_CONSMEM (GPUConstantMem&)(*gGPUConstantMemBuffer)
#endif

__global__ void gHIPMemSetWorkaround(char* ptr, char val, size_t size)
{
  for (size_t i = get_global_id(); i < size; i += get_global_size()) {
    ptr[i] = val;
  }
}

__global__ void dummyInitKernel(void* foo) {}

#if defined(HAVE_O2HEADERS) && !defined(GPUCA_NO_ITS_TRAITS)
#include "ITStrackingHIP/VertexerTraitsHIP.h"
#else
namespace o2
{
namespace its
{
class VertexerTraitsHIP : public VertexerTraits
{
};
class TrackerTraitsHIP : public TrackerTraits
{
};
} // namespace its
} // namespace o2
#endif

#include "GPUReconstructionIncludesDevice.h"

/*
// Not using templated kernel any more, since nvidia profiler does not resolve template names
template <class T, int I, typename... Args>
GPUg() void runKernelHIP(GPUCA_CONSMEM_PTR int iSlice, Args... args)
{
  GPUshared() typename T::GPUSharedMemory smem;
  T::template Thread<I>(get_num_groups(0), get_local_size(0), get_group_id(0), get_local_id(0), smem, T::Processor(GPUCA_CONSMEM)[iSlice], args...);
}
*/

#undef GPUCA_KRNL_REG
#define GPUCA_KRNL_REG(args) __launch_bounds__(GPUCA_M_STRIP(args))
#undef GPUCA_KRNL_BACKEND_XARGS
#define GPUCA_KRNL_BACKEND_XARGS hipEvent_t *start, hipEvent_t *stop,
#define GPUCA_KRNL(x_class, x_attributes, x_arguments, x_forward) GPUCA_KRNL_WRAP(GPUCA_KRNL_, x_class, x_attributes, x_arguments, x_forward)
#define GPUCA_KRNL_BACKEND_CLASS GPUReconstructionHIPBackend
#define GPUCA_KRNL_CALL_single(x_class, x_attributes, x_arguments, x_forward)                                                                                                                                                  \
  if (start == nullptr) {                                                                                                                                                                                                      \
    hipLaunchKernelGGL(HIP_KERNEL_NAME(GPUCA_M_CAT(krnl_, GPUCA_M_KRNL_NAME(x_class))), dim3(x.nBlocks), dim3(x.nThreads), 0, me->mInternals->HIPStreams[x.stream], GPUCA_CONSMEM_CALL y.start, args...);                      \
  } else {                                                                                                                                                                                                                     \
    hipExtLaunchKernelGGL(HIP_KERNEL_NAME(GPUCA_M_CAT(krnl_, GPUCA_M_KRNL_NAME(x_class))), dim3(x.nBlocks), dim3(x.nThreads), 0, me->mInternals->HIPStreams[x.stream], *start, *stop, 0, GPUCA_CONSMEM_CALL y.start, args...); \
  }
#define GPUCA_KRNL_CALL_multi(x_class, x_attributes, x_arguments, x_forward)                                                                                                                                                                   \
  if (start == nullptr) {                                                                                                                                                                                                                      \
    hipLaunchKernelGGL(HIP_KERNEL_NAME(GPUCA_M_CAT3(krnl_, GPUCA_M_KRNL_NAME(x_class), _multi)), dim3(x.nBlocks), dim3(x.nThreads), 0, me->mInternals->HIPStreams[x.stream], GPUCA_CONSMEM_CALL y.start, y.num, args...);                      \
  } else {                                                                                                                                                                                                                                     \
    hipExtLaunchKernelGGL(HIP_KERNEL_NAME(GPUCA_M_CAT3(krnl_, GPUCA_M_KRNL_NAME(x_class), _multi)), dim3(x.nBlocks), dim3(x.nThreads), 0, me->mInternals->HIPStreams[x.stream], *start, *stop, 0, GPUCA_CONSMEM_CALL y.start, y.num, args...); \
  }
#include "GPUReconstructionKernels.h"
#undef GPUCA_KRNL

template <class T, int I, typename... Args>
int GPUReconstructionHIPBackend::runKernelBackend(krnlSetup& _xyz, Args... args)
{
  auto& x = _xyz.x;
  auto& z = _xyz.z;
  if (z.evList) {
    for (int k = 0; k < z.nEvents; k++) {
      GPUFailedMsg(hipStreamWaitEvent(mInternals->HIPStreams[x.stream], ((hipEvent_t*)z.evList)[k], 0));
    }
  }
  hipEvent_t start, stop;
  if (mDeviceProcessingSettings.deviceTimers) {
    GPUFailedMsg(hipEventCreate(&start));
    GPUFailedMsg(hipEventCreate(&stop));
#ifdef __CUDACC__
    GPUFailedMsg(hipEventRecord(start));
#endif
    backendInternal<T, I>::runKernelBackendInternal(_xyz, this, &start, &stop, args...);
#ifdef __CUDACC__
    GPUFailedMsg(hipEventRecord(stop));
#endif
    GPUFailedMsg(hipEventSynchronize(stop));
    float v;
    GPUFailedMsg(hipEventElapsedTime(&v, start, stop));
    _xyz.t = v * 1.e-3;
    GPUFailedMsg(hipEventDestroy(start));
    GPUFailedMsg(hipEventDestroy(stop));
  } else {
    backendInternal<T, I>::runKernelBackendInternal(_xyz, this, nullptr, nullptr, args...);
  }
  GPUFailedMsg(hipGetLastError());
  if (z.ev) {
    GPUFailedMsg(hipEventRecord(*(hipEvent_t*)z.ev, mInternals->HIPStreams[x.stream]));
  }
  return 0;
}

GPUReconstructionHIPBackend::GPUReconstructionHIPBackend(const GPUSettingsProcessing& cfg) : GPUReconstructionDeviceBase(cfg, sizeof(GPUReconstructionDeviceBase))
{
  mInternals = new GPUReconstructionHIPInternals;
  mProcessingSettings.deviceType = DeviceType::HIP;
}

GPUReconstructionHIPBackend::~GPUReconstructionHIPBackend()
{
  Exit(); // Make sure we destroy everything (in particular the ITS tracker) before we exit CUDA
  delete mInternals;
}

GPUReconstruction* GPUReconstruction_Create_HIP(const GPUSettingsProcessing& cfg) { return new GPUReconstructionHIP(cfg); }

void GPUReconstructionHIPBackend::GetITSTraits(std::unique_ptr<o2::its::TrackerTraits>* trackerTraits, std::unique_ptr<o2::its::VertexerTraits>* vertexerTraits)
{
  // if (trackerTraits) {
  //   trackerTraits->reset(new o2::its::TrackerTraitsNV);
  // }
  if (vertexerTraits) {
    vertexerTraits->reset(new o2::its::VertexerTraitsHIP);
  }
}

int GPUReconstructionHIPBackend::InitDevice_Runtime()
{
  // Find best HIP device, initialize and allocate memory
  GPUCA_GPUReconstructionUpdateDefailts();

  hipDeviceProp_t hipDeviceProp;
  int count, bestDevice = -1;
  double bestDeviceSpeed = -1, deviceSpeed;
  if (GPUFailedMsgI(hipGetDeviceCount(&count))) {
    GPUError("Error getting HIP Device Count");
    return (1);
  }
  if (mDeviceProcessingSettings.debugLevel >= 2) {
    GPUInfo("Available HIP devices:");
  }
  const int reqVerMaj = 2;
  const int reqVerMin = 0;
  std::vector<bool> devicesOK(count, false);
  for (int i = 0; i < count; i++) {
    if (mDeviceProcessingSettings.debugLevel >= 4) {
      GPUInfo("Examining device %d", i);
    }
    if (mDeviceProcessingSettings.debugLevel >= 4) {
      GPUInfo("Obtained current memory usage for device %d", i);
    }
    if (GPUFailedMsgI(hipGetDeviceProperties(&hipDeviceProp, i))) {
      continue;
    }
    if (mDeviceProcessingSettings.debugLevel >= 4) {
      GPUInfo("Obtained device properties for device %d", i);
    }
    int deviceOK = true;
    const char* deviceFailure = "";
    if (hipDeviceProp.major >= 9) {
      deviceOK = false;
      deviceFailure = "Invalid Revision";
    } else if (hipDeviceProp.major < reqVerMaj || (hipDeviceProp.major == reqVerMaj && hipDeviceProp.minor < reqVerMin)) {
      deviceOK = false;
      deviceFailure = "Too low device revision";
    }

    deviceSpeed = (double)hipDeviceProp.multiProcessorCount * (double)hipDeviceProp.clockRate * (double)hipDeviceProp.warpSize * (double)hipDeviceProp.major * (double)hipDeviceProp.major;
    if (mDeviceProcessingSettings.debugLevel >= 2) {
      GPUImportant("Device %s%2d: %s (Rev: %d.%d - Mem %lld)%s %s", deviceOK ? " " : "[", i, hipDeviceProp.name, hipDeviceProp.major, hipDeviceProp.minor, (long long int)hipDeviceProp.totalGlobalMem, deviceOK ? " " : " ]", deviceOK ? "" : deviceFailure);
    }
    if (!deviceOK) {
      continue;
    }
    devicesOK[i] = true;
    if (deviceSpeed > bestDeviceSpeed) {
      bestDevice = i;
      bestDeviceSpeed = deviceSpeed;
    } else {
      if (mDeviceProcessingSettings.debugLevel >= 2) {
        GPUInfo("Skipping: Speed %f < %f\n", deviceSpeed, bestDeviceSpeed);
      }
    }
  }
  if (bestDevice == -1) {
    GPUWarning("No %sHIP Device available, aborting HIP Initialisation", count ? "appropriate " : "");
    GPUImportant("Requiring Revision %d.%d, Mem: %lld", reqVerMaj, reqVerMin, (long long int)mDeviceMemorySize);
    return (1);
  }

  if (mDeviceProcessingSettings.deviceNum > -1) {
    if (mDeviceProcessingSettings.deviceNum >= (signed)count) {
      GPUWarning("Requested device ID %d does not exist", mDeviceProcessingSettings.deviceNum);
      return (1);
    } else if (!devicesOK[mDeviceProcessingSettings.deviceNum]) {
      GPUWarning("Unsupported device requested (%d)", mDeviceProcessingSettings.deviceNum);
      return (1);
    } else {
      bestDevice = mDeviceProcessingSettings.deviceNum;
    }
  }
  mDeviceId = bestDevice;

  GPUFailedMsgI(hipGetDeviceProperties(&hipDeviceProp, mDeviceId));
  hipDeviceProp.totalConstMem = 65536; // TODO: Remove workaround, fixes incorrectly reported HIP constant memory

  if (mDeviceProcessingSettings.debugLevel >= 2) {
    GPUInfo("Using HIP Device %s with Properties:", hipDeviceProp.name);
    GPUInfo("\ttotalGlobalMem = %lld", (unsigned long long int)hipDeviceProp.totalGlobalMem);
    GPUInfo("\tsharedMemPerBlock = %lld", (unsigned long long int)hipDeviceProp.sharedMemPerBlock);
    GPUInfo("\tregsPerBlock = %d", hipDeviceProp.regsPerBlock);
    GPUInfo("\twarpSize = %d", hipDeviceProp.warpSize);
    GPUInfo("\tmaxThreadsPerBlock = %d", hipDeviceProp.maxThreadsPerBlock);
    GPUInfo("\tmaxThreadsDim = %d %d %d", hipDeviceProp.maxThreadsDim[0], hipDeviceProp.maxThreadsDim[1], hipDeviceProp.maxThreadsDim[2]);
    GPUInfo("\tmaxGridSize = %d %d %d", hipDeviceProp.maxGridSize[0], hipDeviceProp.maxGridSize[1], hipDeviceProp.maxGridSize[2]);
    GPUInfo("\ttotalConstMem = %lld", (unsigned long long int)hipDeviceProp.totalConstMem);
    GPUInfo("\tmajor = %d", hipDeviceProp.major);
    GPUInfo("\tminor = %d", hipDeviceProp.minor);
    GPUInfo("\tclockRate = %d", hipDeviceProp.clockRate);
    GPUInfo("\tmemoryClockRate = %d", hipDeviceProp.memoryClockRate);
    GPUInfo("\tmultiProcessorCount = %d", hipDeviceProp.multiProcessorCount);
    GPUInfo(" ");
  }
  mCoreCount = hipDeviceProp.multiProcessorCount;
  mDeviceName = hipDeviceProp.name;
  mDeviceName += " (HIP GPU)";

  if (hipDeviceProp.major < 3) {
    GPUError("Unsupported HIP Device");
    return (1);
  }
#ifndef GPUCA_HIP_NO_CONSTANT_MEMORY
  if (gGPUConstantMemBufferSize > hipDeviceProp.totalConstMem) {
    GPUError("Insufficient constant memory available on GPU %d < %d!", (int)hipDeviceProp.totalConstMem, (int)gGPUConstantMemBufferSize);
    return (1);
  }
#endif

  mNStreams = std::max(mDeviceProcessingSettings.nStreams, 3);

  /*if (GPUFailedMsgI(hipDeviceSetLimit(hipLimitStackSize, GPUCA_GPU_STACK_SIZE)))
  {
    GPUError("Error setting HIP stack size");
    GPUFailedMsgI(hipDeviceReset());
    return(1);
  }*/

  if (mDeviceMemorySize > hipDeviceProp.totalGlobalMem || GPUFailedMsgI(hipMalloc(&mDeviceMemoryBase, mDeviceMemorySize))) {
    GPUError("HIP Memory Allocation Error (trying %lld bytes, %lld available)", (long long int)mDeviceMemorySize, (long long int)hipDeviceProp.totalGlobalMem);
    GPUFailedMsgI(hipDeviceReset());
    return (1);
  }
  if (GPUFailedMsgI(hipHostMalloc(&mHostMemoryBase, mHostMemorySize))) {
    GPUError("Error allocating Page Locked Host Memory (trying %lld bytes)", (long long int)mHostMemorySize);
    GPUFailedMsgI(hipDeviceReset());
    return (1);
  }
  if (mDeviceProcessingSettings.debugLevel >= 1) {
    GPUInfo("Memory ptrs: GPU (%lld bytes): %p - Host (%lld bytes): %p", (long long int)mDeviceMemorySize, mDeviceMemoryBase, (long long int)mHostMemorySize, mHostMemoryBase);
    memset(mHostMemoryBase, 0, mHostMemorySize);
    if (GPUFailedMsgI(hipMemset(mDeviceMemoryBase, 0xDD, mDeviceMemorySize))) {
      GPUError("Error during HIP memset, trying workaround with kernel");
      hipLaunchKernelGGL(HIP_KERNEL_NAME(gHIPMemSetWorkaround), dim3(mCoreCount), dim3(256), 0, 0, (char*)mDeviceMemoryBase, 0xDD, mDeviceMemorySize);
      if (GPUFailedMsgI(hipGetLastError()) || GPUFailedMsgI(hipDeviceSynchronize())) {
        GPUError("Error during HIP memset");
        GPUFailedMsgI(hipDeviceReset());
        return (1);
      }
    }
  }

  for (int i = 0; i < mNStreams; i++) {
    if (GPUFailedMsgI(hipStreamCreate(&mInternals->HIPStreams[i]))) {
      GPUError("Error creating HIP Stream");
      GPUFailedMsgI(hipDeviceReset());
      return (1);
    }
  }

  void* devPtrConstantMem;
#ifndef GPUCA_HIP_NO_CONSTANT_MEMORY
  if (GPUFailedMsgI(hipGetSymbolAddress(&devPtrConstantMem, HIP_SYMBOL(gGPUConstantMemBuffer)))) {
    GPUError("Error getting ptr to constant memory");
    GPUFailedMsgI(hipDeviceReset());
    return 1;
  }
#else
  if (GPUFailedMsgI(hipMalloc(&devPtrConstantMem, gGPUConstantMemBufferSize))) {
    GPUError("HIP Memory Allocation Error");
    GPUFailedMsgI(hipDeviceReset());
    return (1);
  }
#endif
  mDeviceConstantMem = (GPUConstantMem*)devPtrConstantMem;

  for (unsigned int i = 0; i < mEvents.size(); i++) {
    hipEvent_t* events = (hipEvent_t*)mEvents[i].data();
    for (unsigned int j = 0; j < mEvents[i].size(); j++) {
      if (GPUFailedMsgI(hipEventCreate(&events[j]))) {
        GPUError("Error creating event");
        GPUFailedMsgI(hipDeviceReset());
        return 1;
      }
    }
  }

  hipLaunchKernelGGL(HIP_KERNEL_NAME(dummyInitKernel), dim3(mCoreCount), dim3(256), 0, 0, mDeviceMemoryBase);
  GPUInfo("HIP Initialisation successfull (Device %d: %s (Frequency %d, Cores %d), %lld / %lld bytes host / global memory, Stack frame %d, Constant memory %lld)", mDeviceId, hipDeviceProp.name, hipDeviceProp.clockRate, hipDeviceProp.multiProcessorCount, (long long int)mHostMemorySize,
          (long long int)mDeviceMemorySize, (int)GPUCA_GPU_STACK_SIZE, (long long int)gGPUConstantMemBufferSize);

  return (0);
}

int GPUReconstructionHIPBackend::ExitDevice_Runtime()
{
  // Uninitialize HIP
  SynchronizeGPU();

  GPUFailedMsgI(hipFree(mDeviceMemoryBase));
  mDeviceMemoryBase = nullptr;
#ifdef GPUCA_HIP_NO_CONSTANT_MEMORY
  GPUFailedMsgI(hipFree(mDeviceConstantMem));
#endif

  for (int i = 0; i < mNStreams; i++) {
    GPUFailedMsgI(hipStreamDestroy(mInternals->HIPStreams[i]));
  }

  GPUFailedMsgI(hipHostFree(mHostMemoryBase));
  mHostMemoryBase = nullptr;

  for (unsigned int i = 0; i < mEvents.size(); i++) {
    hipEvent_t* events = (hipEvent_t*)mEvents[i].data();
    for (unsigned int j = 0; j < mEvents[i].size(); j++) {
      GPUFailedMsgI(hipEventDestroy(events[j]));
    }
  }

  if (GPUFailedMsgI(hipDeviceReset())) {
    GPUError("Could not uninitialize GPU");
    return (1);
  }

  GPUInfo("HIP Uninitialized");
  return (0);
}

size_t GPUReconstructionHIPBackend::GPUMemCpy(void* dst, const void* src, size_t size, int stream, bool toGPU, deviceEvent* ev, deviceEvent* evList, int nEvents)
{
  if (mDeviceProcessingSettings.debugLevel >= 3) {
    stream = -1;
  }
  if (stream == -1) {
    SynchronizeGPU();
    GPUFailedMsg(hipMemcpy(dst, src, size, toGPU ? hipMemcpyHostToDevice : hipMemcpyDeviceToHost));
  } else {
    if (evList == nullptr) {
      nEvents = 0;
    }
    for (int k = 0; k < nEvents; k++) {
      GPUFailedMsg(hipStreamWaitEvent(mInternals->HIPStreams[stream], ((hipEvent_t*)evList)[k], 0));
    }
    GPUFailedMsg(hipMemcpyAsync(dst, src, size, toGPU ? hipMemcpyHostToDevice : hipMemcpyDeviceToHost, mInternals->HIPStreams[stream]));
  }
  if (ev) {
    GPUFailedMsg(hipEventRecord(*(hipEvent_t*)ev, mInternals->HIPStreams[stream == -1 ? 0 : stream]));
  }
  return size;
}

size_t GPUReconstructionHIPBackend::TransferMemoryInternal(GPUMemoryResource* res, int stream, deviceEvent* ev, deviceEvent* evList, int nEvents, bool toGPU, const void* src, void* dst)
{
  if (!(res->Type() & GPUMemoryResource::MEMORY_GPU)) {
    if (mDeviceProcessingSettings.debugLevel >= 4) {
      GPUInfo("Skipped transfer of non-GPU memory resource: %s", res->Name());
    }
    return 0;
  }
  if (mDeviceProcessingSettings.debugLevel >= 3) {
    GPUInfo(toGPU ? "Copying to GPU: %s\n" : "Copying to Host: %s", res->Name());
  }
  return GPUMemCpy(dst, src, res->Size(), stream, toGPU, ev, evList, nEvents);
}

size_t GPUReconstructionHIPBackend::WriteToConstantMemory(size_t offset, const void* src, size_t size, int stream, deviceEvent* ev)
{
#ifndef GPUCA_HIP_NO_CONSTANT_MEMORY
  if (stream == -1) {
    GPUFailedMsg(hipMemcpyToSymbol(HIP_SYMBOL(gGPUConstantMemBuffer), src, size, offset, hipMemcpyHostToDevice));
  } else {
    GPUFailedMsg(hipMemcpyToSymbolAsync(HIP_SYMBOL(gGPUConstantMemBuffer), src, size, offset, hipMemcpyHostToDevice, mInternals->HIPStreams[stream]));
  }
  if (ev && stream != -1) {
    GPUFailedMsg(hipEventRecord(*(hipEvent_t*)ev, mInternals->HIPStreams[stream]));
  }

#else
  if (stream == -1) {
    GPUFailedMsg(hipMemcpy(((char*)mDeviceConstantMem) + offset, src, size, hipMemcpyHostToDevice));
  } else {
    GPUFailedMsg(hipMemcpyAsync(((char*)mDeviceConstantMem) + offset, src, size, hipMemcpyHostToDevice, mInternals->HIPStreams[stream]));
  }

#endif
  return size;
}

void GPUReconstructionHIPBackend::ReleaseEvent(deviceEvent* ev) {}

void GPUReconstructionHIPBackend::RecordMarker(deviceEvent* ev, int stream) { GPUFailedMsg(hipEventRecord(*(hipEvent_t*)ev, mInternals->HIPStreams[stream])); }

void GPUReconstructionHIPBackend::SynchronizeGPU() { GPUFailedMsg(hipDeviceSynchronize()); }

void GPUReconstructionHIPBackend::SynchronizeStream(int stream) { GPUFailedMsg(hipStreamSynchronize(mInternals->HIPStreams[stream])); }

void GPUReconstructionHIPBackend::SynchronizeEvents(deviceEvent* evList, int nEvents)
{
  for (int i = 0; i < nEvents; i++) {
    GPUFailedMsg(hipEventSynchronize(((hipEvent_t*)evList)[i]));
  }
}

bool GPUReconstructionHIPBackend::IsEventDone(deviceEvent* evList, int nEvents)
{
  for (int i = 0; i < nEvents; i++) {
    hipError_t retVal = hipEventSynchronize(((hipEvent_t*)evList)[i]);
    if (retVal == hipErrorNotReady) {
      return false;
    }
    GPUFailedMsg(retVal);
  }
  return (true);
}

int GPUReconstructionHIPBackend::GPUDebug(const char* state, int stream)
{
  // Wait for HIP-Kernel to finish and check for HIP errors afterwards, in case of debugmode
  if (mDeviceProcessingSettings.debugLevel == 0) {
    return (0);
  }
  hipError_t cuErr;
  cuErr = hipGetLastError();
  if (cuErr != hipSuccess) {
    GPUError("HIP Error %s while running kernel (%s) (Stream %d)", hipGetErrorString(cuErr), state, stream);
    return (1);
  }
  if (GPUFailedMsgI(hipDeviceSynchronize())) {
    GPUError("HIP Error while synchronizing (%s) (Stream %d)", state, stream);
    return (1);
  }
  if (mDeviceProcessingSettings.debugLevel >= 3) {
    GPUInfo("GPU Sync Done");
  }
  return (0);
}

void GPUReconstructionHIPBackend::SetThreadCounts()
{
  mThreadCount = GPUCA_THREAD_COUNT;
  mBlockCount = mCoreCount;
  mConstructorBlockCount = mBlockCount * (mDeviceProcessingSettings.trackletConstructorInPipeline ? 1 : GPUCA_BLOCK_COUNT_CONSTRUCTOR_MULTIPLIER);
  mSelectorBlockCount = mBlockCount * GPUCA_BLOCK_COUNT_SELECTOR_MULTIPLIER;
  mConstructorThreadCount = GPUCA_THREAD_COUNT_CONSTRUCTOR;
  mSelectorThreadCount = GPUCA_THREAD_COUNT_SELECTOR;
  mFinderThreadCount = GPUCA_THREAD_COUNT_FINDER;
  mTRDThreadCount = GPUCA_THREAD_COUNT_TRD;
  mClustererThreadCount = GPUCA_THREAD_COUNT_CLUSTERER;
  mScanThreadCount = GPUCA_THREAD_COUNT_SCAN;
  mConverterThreadCount = GPUCA_THREAD_COUNT_CONVERTER;
  mCompression1ThreadCount = GPUCA_THREAD_COUNT_COMPRESSION1;
  mCompression2ThreadCount = GPUCA_THREAD_COUNT_COMPRESSION2;
  mCFDecodeThreadCount = GPUCA_THREAD_COUNT_CFDECODE;
  mFitThreadCount = GPUCA_THREAD_COUNT_FIT;
  mITSThreadCount = GPUCA_THREAD_COUNT_ITS;
  mWarpSize = GPUCA_WARP_SIZE;
}

int GPUReconstructionHIPBackend::registerMemoryForGPU(void* ptr, size_t size)
{
  return GPUFailedMsgI(hipHostRegister(ptr, size, hipHostRegisterDefault));
}

int GPUReconstructionHIPBackend::unregisterMemoryForGPU(void* ptr)
{
  return GPUFailedMsgI(hipHostUnregister(ptr));
}
