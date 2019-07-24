// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// \file PatchPanel.h
/// \brief MFT Patch Panel builder

#ifndef ALICEO2_MFT_PATCHPANEL_H_
#define ALICEO2_MFT_PATCHPANEL_H_

#include "TGeoVolume.h"
#include "TGeoMatrix.h"

class TGeoVolumeAssembly;

namespace o2
{
namespace mft
{

class PatchPanel
{

 public:
  PatchPanel();
  ~PatchPanel() = default;

  TGeoVolumeAssembly* createPatchPanel();

  //protected:
  //TGeoVolumeAssembly* mPatchPanel;

 private:
  ClassDef(PatchPanel, 1)
  //ClassDefOverride(PatchPanel, 1)
};
} // namespace mft
} // namespace o2

#endif
