

#ifndef ALICEO2_MFT_HALFSUPPORT_H_
#define ALICEO2_MFT_HALFSUPPORT_H_

#include "TNamed.h"

class TGeoVolumeAssembly;

namespace o2
{
namespace MFT 
{

class HalfSupport : public TNamed
{
  
public:
  
  HalfSupport();
  
  ~HalfSupport() override;
  
  TGeoVolumeAssembly* createHalfSupport(Int_t half);

protected:

  TGeoVolumeAssembly * mHalfSupport;

private:
  
  ClassDefOverride(HalfSupport,1)
  
};

}
}

#endif

