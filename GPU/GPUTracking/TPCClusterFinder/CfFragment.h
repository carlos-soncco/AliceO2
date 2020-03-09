// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// \file CfFragment.h
/// \author Felix Weiglhofer

#ifndef O2_GPU_CF_FRAGMENT_H
#define O2_GPU_CF_FRAGMENT_H

#include "clusterFinderDefs.h"
#include "GPUCommonMath.h"

namespace GPUCA_NAMESPACE
{
namespace gpu
{

struct CfFragment {

  enum : TPCTime {
    OverlapTimebins = 8,
  };

  // Time offset of this sub slice within the entire time slice
  TPCTime start = 0;
  // Number of time bins to process in this slice
  TPCFragmentTime length = 0;

  size_t digitsStart = 0; // Start digits in this fragment. Only used when zero suppression is skipped

  unsigned int index = 0;

  bool hasBacklog = false;
  bool hasFuture = false;

  GPUdDefault() CfFragment() CON_DEFAULT;

  GPUhd() CfFragment(TPCTime totalSliceLen, TPCFragmentTime maxSubSliceLen) : CfFragment(0, false, 0, totalSliceLen, maxSubSliceLen) {}

  GPUhdi() bool isEnd() const { return length == 0; }

  GPUhdi() CfFragment next(TPCTime totalSliceLen, TPCFragmentTime maxSubSliceLen) const
  {
    return CfFragment{index + 1, hasFuture, TPCTime(start + length - (hasFuture ? 2 * OverlapTimebins : 0)), totalSliceLen, maxSubSliceLen};
  }

  GPUhdi() TPCTime first() const
  {
    return start;
  }

  GPUhdi() TPCTime last() const
  {
    return start + length;
  }

  GPUhdi() bool contains(TPCTime t) const
  {
    return first() <= t && t < last();
  }

  // Wether a timebin falls into backlog or future
  GPUhdi() bool isOverlap(TPCFragmentTime t) const
  {
    return (hasBacklog ? t < OverlapTimebins : false) || (hasFuture ? t >= (length - OverlapTimebins) : false);
  }

  GPUhdi() TPCFragmentTime toLocal(TPCTime t) const
  {
    return t - first();
  }

  GPUhdi() TPCTime toGlobal(TPCFragmentTime t) const
  {
    return t + first();
  }

 private:
  GPUhd() CfFragment(uint index_, bool hasBacklog_, TPCTime start_, TPCTime totalSliceLen, TPCFragmentTime maxSubSliceLen)
  {
    this->index = index_;
    this->hasBacklog = hasBacklog_;
    this->start = start_;

    TPCTime remainder = totalSliceLen - start;
    this->hasFuture = remainder > TPCTime(maxSubSliceLen);
    this->length = hasFuture ? maxSubSliceLen : remainder;
  }
};

} // namespace gpu
} // namespace GPUCA_NAMESPACE

#endif
