// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.
///
/// \file Cluster.cxx
///

#include "MFTTracking/Cluster.h"
#include "MFTTracking/IndexTableUtils.h"
#include "MFTTracking/MathUtils.h"

namespace o2
{
namespace MFT
{

using MathUtils::calculatePhiCoordinate;
using MathUtils::calculateRCoordinate;
using MathUtils::getNormalizedPhiCoordinate;

Cluster::Cluster(const Float_t x, const Float_t y, const Float_t z, const Int_t index)
  : xCoordinate{ x },
    yCoordinate{ y },
    zCoordinate{ z },
    phiCoordinate{ getNormalizedPhiCoordinate(calculatePhiCoordinate(x, y)) },
    rCoordinate{ calculateRCoordinate(x, y) },
    clusterId{ index },
    indexTableBin{ 0 }
{
  // Nothing to do
}

Cluster::Cluster(const Int_t layerIndex, const Cluster& other)
  : xCoordinate{ other.xCoordinate },
    yCoordinate{ other.yCoordinate },
    zCoordinate{ other.zCoordinate },
    phiCoordinate{ getNormalizedPhiCoordinate(calculatePhiCoordinate(other.xCoordinate, other.yCoordinate)) },
    rCoordinate{ calculateRCoordinate(other.xCoordinate, other.yCoordinate) },
    clusterId{ other.clusterId },
    indexTableBin{ IndexTableUtils::getBinIndex(IndexTableUtils::getRBinIndex(layerIndex, rCoordinate),
                                                IndexTableUtils::getPhiBinIndex(phiCoordinate)) }
{
  // Nothing to do
}

} // namespace MFT
} // namespace o2
