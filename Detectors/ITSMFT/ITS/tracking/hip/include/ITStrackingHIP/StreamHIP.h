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
/// \file StreamHIP.h
/// \brief
///

#ifndef O2_ITS_TRACKING_INCLUDE_STREAM_HIP_H_
#define O2_ITS_TRACKING_INCLUDE_STREAM_HIP_H_

#include "ITStracking/Definitions.h"

namespace o2
{
namespace its
{
namespace GPU
{

class Stream final
{

 public:
  Stream();
  ~Stream();

  Stream(const Stream&) = delete;
  Stream& operator=(const Stream&) = delete;

  const GPUStream& get() const;

 private:
  GPUStream mStream;
};
} // namespace GPU
} // namespace its
} // namespace o2

#endif /* O2_ITS_TRACKING_INCLUDE_STREAM_HIP_H_ */
