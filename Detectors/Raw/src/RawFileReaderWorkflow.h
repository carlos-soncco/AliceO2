// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_RAWFILE_READER_WORKFLOW_H
#define O2_RAWFILE_READER_WORKFLOW_H

/// @file   RawFileReaderWorkflow.h

#include "Framework/WorkflowSpec.h"
class string;

namespace o2
{
namespace raw
{

framework::WorkflowSpec getRawFileReaderWorkflow(std::string inifile, int loop = 1, uint32_t delay_us = 0, uint32_t errMap = 0xffffffff,
                                                 uint32_t minTF = 0, uint32_t maxTF = 0xffffffff, bool partPerSP = true, bool cache = false, bool autodetectTF0 = false,
                                                 size_t spSize = 1024L * 1024L, size_t bufferSize = 1024L * 1024L,
                                                 const std::string& rawChannelConfig = "");

} // namespace raw
} // namespace o2
#endif
