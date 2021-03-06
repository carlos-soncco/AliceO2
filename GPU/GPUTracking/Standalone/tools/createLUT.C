#if !defined(__CLING__) || defined(__ROOTCLING__)
#include <TSystem.h>
#include "DetectorsBase/MatLayerCylSet.h"
#include "GPUO2Interface.h"
#include "GPUReconstruction.h"
#include "GPUChainTracking.h"

using namespace GPUCA_NAMESPACE::gpu;

void createLUT()
{
  o2::base::MatLayerCylSet* lut = o2::base::MatLayerCylSet::loadFromFile("matbud.root", "MatBud");
  gSystem->Load("libO2GPUTracking.so");
  GPUReconstruction* rec = GPUReconstruction::CreateInstance(GPUReconstruction::DeviceType::CPU);
  GPUChainTracking* chain = rec->AddChain<GPUChainTracking>();
  chain->SetMatLUT(lut);
  rec->DumpSettings();
}

#endif
