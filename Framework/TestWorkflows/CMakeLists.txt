# Copyright CERN and copyright holders of ALICE O2. This software is
# distributed under the terms of the GNU General Public License v3 (GPL
# Version 3), copied verbatim in the file "COPYING".
#
# See http://alice-o2.web.cern.ch/ for full licensing information.
#
# In applying this license CERN does not waive the privileges and immunities
# granted to it by virtue of its status as an Intergovernmental Organization
# or submit itself to any jurisdiction.

set(MODULE_NAME "TestWorkflows")
set(MODULE_BUCKET_NAME FrameworkApplication_bucket)

O2_SETUP(NAME ${MODULE_NAME})
set(SRCS
    src/dummy.cxx
   )

set(HEADERS
    src/o2_sim_its_ALP3.h
    src/o2_sim_tpc.h
  )

## TODO: feature of macro, it deletes the variables we pass to it, set them again
## this has to be fixed in the macro implementation
set(LIBRARY_NAME ${MODULE_NAME})
set(BUCKET_NAME ${MODULE_BUCKET_NAME})

O2_GENERATE_LIBRARY()

O2_GENERATE_EXECUTABLE(
  EXE_NAME "o2-dummy-workflow"
  SOURCES "src/o2DummyWorkflow.cxx"
  MODULE_LIBRARY_NAME ${LIBRARY_NAME}
  BUCKET_NAME ${MODULE_BUCKET_NAME}
)

O2_GENERATE_EXECUTABLE(
  EXE_NAME "test_o2RootMessageWorkflow"
  SOURCES "src/test_o2RootMessageWorkflow.cxx"
  MODULE_LIBRARY_NAME ${LIBRARY_NAME}
  BUCKET_NAME ${MODULE_BUCKET_NAME}
)

O2_GENERATE_EXECUTABLE(
  EXE_NAME "o2-diamond-workflow"
  SOURCES "src/o2DiamondWorkflow.cxx"
  MODULE_LIBRARY_NAME ${LIBRARY_NAME}
  BUCKET_NAME ${MODULE_BUCKET_NAME}
)

O2_GENERATE_EXECUTABLE(
  EXE_NAME "o2-parallel-workflow"
  SOURCES "src/o2ParallelWorkflow.cxx"
  MODULE_LIBRARY_NAME ${LIBRARY_NAME}
  BUCKET_NAME ${MODULE_BUCKET_NAME}
)

O2_GENERATE_EXECUTABLE(
  EXE_NAME "o2-flp-qualification"
  SOURCES "src/flpQualification.cxx"
  MODULE_LIBRARY_NAME ${LIBRARY_NAME}
  BUCKET_NAME ${MODULE_BUCKET_NAME}
)

O2_GENERATE_EXECUTABLE(
  EXE_NAME "o2-sync-reconstruction-dummy"
  SOURCES "src/o2SyncReconstructionDummy.cxx"
  MODULE_LIBRARY_NAME ${LIBRARY_NAME}
  BUCKET_NAME ${MODULE_BUCKET_NAME}
)

O2_GENERATE_EXECUTABLE(
  EXE_NAME "o2-aod-dummy-workflow"
  SOURCES "src/o2AODDummy.cxx"
  MODULE_LIBRARY_NAME ${LIBRARY_NAME}
  BUCKET_NAME ${MODULE_BUCKET_NAME}
)

O2_GENERATE_EXECUTABLE(
  EXE_NAME "o2-d0-analysis"
  SOURCES "src/o2D0Analysis.cxx"
  MODULE_LIBRARY_NAME ${LIBRARY_NAME}
  BUCKET_NAME ${MODULE_BUCKET_NAME}
)

O2_GENERATE_EXECUTABLE(
  EXE_NAME "o2-simple-tracks-analysis"
  SOURCES "src/o2SimpleTracksAnalysis.cxx"
  MODULE_LIBRARY_NAME ${LIBRARY_NAME}
  BUCKET_NAME ${MODULE_BUCKET_NAME}
)

O2_GENERATE_EXECUTABLE(
  EXE_NAME "o2-analysis-task-example"
  SOURCES "src/o2AnalysisTaskExample.cxx"
  MODULE_LIBRARY_NAME ${LIBRARY_NAME}
  BUCKET_NAME ${MODULE_BUCKET_NAME}
)

O2_GENERATE_EXECUTABLE(
  EXE_NAME "o2-data-query-workflow"
  SOURCES "src/o2DataQueryWorkflow.cxx"
  MODULE_LIBRARY_NAME ${LIBRARY_NAME}
  BUCKET_NAME ${MODULE_BUCKET_NAME}
)

O2_GENERATE_EXECUTABLE(
  EXE_NAME "test_MakeDPLObjects"
  SOURCES "test/test_MakeDPLObjects.cxx"
  MODULE_LIBRARY_NAME ${LIBRARY_NAME}
  BUCKET_NAME ${MODULE_BUCKET_NAME}
)

O2_GENERATE_EXECUTABLE(
  EXE_NAME "test_RawDeviceInjector"
  SOURCES "src/test_RawDeviceInjector.cxx"
  MODULE_LIBRARY_NAME ${LIBRARY_NAME}
  BUCKET_NAME ${MODULE_BUCKET_NAME}
)

O2_GENERATE_EXECUTABLE(
  EXE_NAME "test_CompletionPolicies"
  SOURCES "src/test_CompletionPolicies.cxx"
  MODULE_LIBRARY_NAME ${LIBRARY_NAME}
  BUCKET_NAME ${MODULE_BUCKET_NAME}
)

O2_GENERATE_EXECUTABLE(
  EXE_NAME "o2-datasampling-pod-and-root"
  SOURCES "src/dataSamplingPodAndRoot.cxx"
  MODULE_LIBRARY_NAME ${LIBRARY_NAME}
  BUCKET_NAME ${MODULE_BUCKET_NAME}
)

O2_GENERATE_EXECUTABLE(
  EXE_NAME "o2-datasampling-parallel"
  SOURCES "src/dataSamplingParallel.cxx"
  MODULE_LIBRARY_NAME ${LIBRARY_NAME}
  BUCKET_NAME ${MODULE_BUCKET_NAME}
)

O2_GENERATE_EXECUTABLE(
  EXE_NAME "o2-datasampling-time-pipeline"
  SOURCES "src/dataSamplingTimePipeline.cxx"
  MODULE_LIBRARY_NAME ${LIBRARY_NAME}
  BUCKET_NAME ${MODULE_BUCKET_NAME}
)

# These should be enabled only if one uses the full O2 default
if (PYTHIA8_INCLUDE_DIR)
  O2_FRAMEWORK_WORKFLOW(
    WORKFLOW_NAME "o2ITSClusterizers"
    DETECTOR_BUCKETS its_simulation_bucket
                     its_reconstruction_bucket
                     passive_detector_bucket
                     generators_base_bucket
                     generators_bucket
                     run_bucket
    SOURCES src/test_o2ITSCluserizer.cxx src/o2_sim_its_ALP3.cxx
  )

  O2_FRAMEWORK_WORKFLOW(
    WORKFLOW_NAME "o2TPCSimulation"
    DETECTOR_BUCKETS tpc_simulation_bucket
                     tpc_reconstruction_bucket
                     passive_detector_bucket
                     generators_base_bucket
                     generators_bucket
                     run_bucket
    SOURCES src/test_o2TPCSimulation.cxx src/o2_sim_tpc.cxx
  )
ENDIF(PYTHIA8_INCLUDE_DIR)
