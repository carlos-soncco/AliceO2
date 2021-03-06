// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include <TGeoManager.h>
#include <TRandom.h>

#include "FairLogger.h"
#include "DetectorsBase/GeometryManager.h"

#include "TRDBase/TRDGeometry.h"
#include "TRDBase/TRDCommonParam.h" // For kNdet & el. diffusion
#include "TRDBase/TRDSimParam.h"
#include "TRDBase/TRDPadPlane.h"
#include "TRDBase/TRDArraySignal.h"
#include "TRDBase/PadResponse.h"

#include "TRDSimulation/Digitizer.h"

using namespace o2::trd;

Digitizer::Digitizer()
{
  // Check if you need more initialization
  o2::base::GeometryManager::loadGeometry();
  mGeo = new TRDGeometry();
  mGeo->createClusterMatrixArray(); // Requiered for chamberInGeometry()

  mPRF = new PadResponse();

  // get the Instance of simulation and common parameters
  mSimParam = TRDSimParam::Instance();
  mCommonParam = TRDCommonParam::Instance();
  // mCalib = TRDCalibDB::Instace(); // PLEASE FIX ME when CCDB is ready
  if (!mSimParam) {
    LOG(FATAL) << "TRD Simulation Parameters not available";
  }
  if (!mCommonParam) {
    LOG(FATAL) << "TRD Common Parameters not available";
  } else {
    if (!mCommonParam->cacheMagField()) {
      LOG(FATAL) << "TRD Common Parameters does not have magnetic field available";
    }
  }
  // if (!mCalib) { // PLEASE FIX ME when CCDB is ready
  //   LOG(FATAL) << "TRD mCalib database not available";
  // }
  mSDigits = false;
}

void Digitizer::process(std::vector<HitType> const& hits, std::vector<Digit>& digits)
{
  // (WIP) Implementation for digitization

  // Check if Geometry and if CCDB are available as they will be requiered
  // const int nTimeBins = mCalib->GetNumberOfTimeBinsDCS(); PLEASE FIX ME when CCDB is ready

  // Loop over all TRD detectors
  // Get the a hit container for all the hits in a given detector then call convertHits for a given detector (0 - 539)
  int totalNumberOfProcessedHits = 0;
  // LOG(INFO) << "Start of processing " << hits.size() << " hits";

  for (int det = 0; det < kNdet; ++det) {
    // Loop over all TRD detectors

    // Jump to the next detector if the detector is
    // switched off, not installed, etc
    /*      
    if (mCalib->IsChamberNoData(det)) { // PLEASE FIX ME when CCDB is ready
      continue;
    } */
    if (!mGeo->chamberInGeometry(det)) {
      continue;
    }

    mHitContainer.clear();
    // Skip detectors without hits
    if (!getHitContainer(det, hits, mHitContainer)) {
      // move to next det if no hits are found for this det
      continue;
    }
    totalNumberOfProcessedHits += mHitContainer.size();
    TRDArraySignal signals;
    if (!convertHits(det, mHitContainer, signals)) {
      LOG(WARNING) << "TRD converstion of hits failed for detector " << det;
      signals.reset(); // make sure you have nothing
    }

    digits.emplace_back();
  } // end of loop over detectors
  // LOG(INFO) << "End of processing " << totalNumberOfProcessedHits << " hits";
}

bool Digitizer::getHitContainer(const int det, const std::vector<HitType>& hits, std::vector<HitType>& hitContainer)
{
  //
  // Fills the hit vector for hits in detector number det
  // Returns false if there are no hits in the dectector
  //
  for (const auto& hit : hits) {
    if (hit.GetDetectorID() == det) {
      hitContainer.push_back(hit);
    }
  }
  if (hitContainer.size() == 0) {
    return false;
  }
  return true;
}

bool Digitizer::convertHits(const int det, const std::vector<HitType>& hits, TRDArraySignal& arraySignal)
{
  //
  // Convert the detector-wise sorted hits to detector signals
  //
  const int kNpad = mSimParam->getNumberOfPadsInPadResponse(); // Number of pads included in the pad response
  const float kAmWidth = TRDGeometry::amThick();               // Width of the amplification region
  const float kDrWidth = TRDGeometry::drThick();               // Width of the drift retion
  const float kDrMin = -0.5 * kAmWidth;                        // Drift + Amplification region
  const float kDrMax = kDrWidth + 0.5 * kAmWidth;              // Drift + Amplification region

  int timeBinTRFend = 0;

  double pos[3];
  double loc[3];
  double padSignal[kNpad];
  double signalOld[kNpad];

  // Get the detector wise mCalib objects
  // const TRDCalDet* calVdriftDet = mCalib->GetVdriftDet();    PLEASE FIX ME when CCDB is ready
  // const TRDCalDet* calT0Det = mCalib->GetT0Det();            PLEASE FIX ME when CCDB is ready
  // const TRDCalDet* calExBDet = mCalib->GetExBDet();          PLEASE FIX ME when CCDB is ready

  // FIX ME: Default values until I have implemented the mCalib objects
  //
  // See Table 8 (Nuclear Inst. and Methods in Physics Research, A 881 (2018) 88-127)
  // Defaults values  from OCDB (AliRoot DrawTrending macro - Thanks to Y. Pachmayer)
  // For 5 TeV pp - 27 runs from LHC15n
  //
  float calVdriftDetValue = 1.48; // cm/microsecond         // calVdriftDet->GetValue(det); PLEASE FIX ME when CCDB is ready
  float calT0DetValue = -1.38;    // microseconds           // calT0Det->GetValue(det);     PLEASE FIX ME when CCDB is ready
  double calExBDetValue = 0.16;   // T * V/cm (check units) // calExBDet->GetValue(det);    PLEASE FIX ME when CCDB is ready

  // TRDCalROC* calVdriftROC = mCalib->GetVdriftROC(det); PLEASE FIX ME when CCDB is ready
  // TRDCalROC* calT0ROC = mCalib->GetT0ROC(det);         PLEASE FIX ME when CCDB is ready

  if (mSimParam->TRFOn()) {
    timeBinTRFend = ((int)(mSimParam->GetTRFhi() * mCommonParam->GetSamplingFrequency())) - 1;
  }

  const int nTimeTotal = kTimeBins; // DigitsManager->GetDigitsParam()->GetNTimeBins(det);
  const float samplingRate = mCommonParam->GetSamplingFrequency();
  const float elAttachProp = mSimParam->GetElAttachProp() / 100;

  const TRDPadPlane* padPlane = mGeo->getPadPlane(det);
  const int layer = mGeo->getLayer(det);
  const float rowEndROC = padPlane->getRowEndROC();
  const float row0 = padPlane->getRow0ROC();
  const int nRowMax = padPlane->getNrows();
  const int nColMax = padPlane->getNcols();

  // Allocate space for signals
  // arraySignal.allocate(nRowMax, nColMax, nTimeTotal);

  // Loop over hits
  for (const auto& hit : hits) {
    pos[0] = hit.GetX();
    pos[1] = hit.GetY();
    pos[2] = hit.GetZ();
    gGeoManager->SetCurrentPoint(pos);
    gGeoManager->FindNode();
    // Go to the local coordinate system
    // loc [0] -  col direction in amplification or drift volume
    // loc [1] -  row direction in amplification or drift volume
    // loc [2] -  time direction in amplification or drift volume
    gGeoManager->MasterToLocal(pos, loc);

    const int qTotal = hit.GetCharge();

    const int inDrift = std::strstr(gGeoManager->GetPath(), "/UK") ? 0 : 1;
    if (inDrift) {
      loc[2] = loc[2] - kDrWidth / 2 - kAmWidth / 2;
    }

    const double driftLength = -1 * loc[2]; // The drift length in cm without diffusion

    // Patch to take care of TR photons that are absorbed
    // outside the chamber volume. A real fix would actually need
    // a more clever implementation of the TR hit generation
    if (qTotal < 0) {
      if ((loc[1] < rowEndROC) ||
          (loc[1] > row0)) {
        continue;
      }
      if ((driftLength < kDrMin) ||
          (driftLength > kDrMax)) {
        continue;
      }
    }

    int rowE = padPlane->getPadRowNumberROC(loc[1]);
    if (rowE < 0) {
      continue;
    }

    double rowOffset = padPlane->getPadRowOffsetROC(rowE, loc[1]);
    double offsetTilt = padPlane->getTiltOffset(rowOffset);
    int colE = padPlane->getPadColNumber(loc[0] + offsetTilt);
    if (colE < 0) {
      continue;
    }

    // FIX ME: Commented out what is still not yet implemented
    double absDriftLength = abs(driftLength); // Normalized drift length
    if (mCommonParam->ExBOn()) {
      absDriftLength /= TMath::Sqrt(1 / (1 + calExBDetValue * calExBDetValue));
    }
    // double driftVelocity = calVdriftDetValue * calVdriftROC->GetValue(colE, rowE); PLEASE FIX ME when CCDB is ready
    double driftVelocity = 2.13; // Defaults values  from OCDB (AliRoot DrawTrending macro) for 5 TeV pp - 27 runs from LHC15n

    // Loop over all created electrons
    const int nElectrons = abs(qTotal);
    for (int el = 0; el < nElectrons; ++el) {
      /*
      Now the real local coordinate system of the ROC
      column direction: locC
      row direction:    locR
      time direction:   locT
      locR and locC are identical to the coordinates of the corresponding
      volumina of the drift or amplification region.
      locT is defined relative to the wire plane (i.e. middle of amplification
      region), meaning locT = 0, and is negative for hits coming from the
      drift region.
      */
      double locC = loc[0];
      double locR = loc[1];
      double locT = loc[2];

      // Electron attachment
      if (mSimParam->ElAttachOn()) {
        if (gRandom->Rndm() < absDriftLength * elAttachProp) {
          continue;
        }
      }

      // Apply diffusion smearing
      if (mSimParam->DiffusionOn()) {
        if (!diffusion(driftVelocity, absDriftLength, calExBDetValue, locR, locC, locT)) {
          continue;
        }
      }

      // Apply E x B effects
      if (mCommonParam->ExBOn()) {
        locC = locC + calExBDetValue * driftLength;
      }

      // The electron position after diffusion and ExB in pad coordinates.
      rowE = padPlane->getPadRowNumberROC(locR);
      if (rowE < 1) {
        continue;
      }
      rowOffset = padPlane->getPadRowOffsetROC(rowE, locR);

      // The pad column (rphi-direction)
      offsetTilt = padPlane->getTiltOffset(rowOffset);
      colE = padPlane->getPadColNumber(locC + offsetTilt);
      if (colE < 0) {
        continue;
      }
      const double colOffset = padPlane->getPadColOffset(colE, locC + offsetTilt);

      // Retrieve drift velocity becuase col and row may have changed
      // driftVelocity = calVdriftDetValue* calVdriftROC->GetValue(colE, rowE);  PLEASE FIX ME when CCDB is ready
      driftVelocity = 2.13; // Defaults values  from OCDB (AliRoot DrawTrending macro) for 5 TeV pp - 27 runs from LHC15n
      // float t0 = calT0DetValue + calT0ROC->getValue(colE, rowE);      PLEASE FIX ME when CCDB is ready
      const float t0 = -1.38 + 0; // Defaults values  from OCDB (AliRoot DrawTrending macro) for 5 TeV pp - 27 runs from LHC15n

      // Convert the position to drift time [mus], using either constant drift velocity or
      // time structure of drift cells (non-isochronity, GARFIELD calculation).
      // Also add absolute time of hits to take pile-up events into account properly
      double driftTime;
      if (mSimParam->TimeStructOn()) {
        // Get z-position with respect to anode wire
        double zz = row0 - locR + padPlane->getAnodeWireOffset();
        zz -= ((int)(2 * zz)) * 0.5;
        if (zz > 0.25) {
          zz = 0.5 - zz;
        }
        // Use drift time map (GARFIELD)
        driftTime = mCommonParam->TimeStruct(driftVelocity, 0.5 * kAmWidth - 1.0 * locT, zz) + hit.GetTime();
      } else {
        // Use constant drift velocity
        driftTime = abs(locT) / driftVelocity + hit.GetTime();
      }

      // Apply the gas gain including fluctuations
      double ggRndm = 0;
      do {
        ggRndm = gRandom->Rndm();
      } while (ggRndm <= 0);
      double signal = -(mSimParam->GetGasGain()) * TMath::Log(ggRndm);

      // Apply the pad response
      if (mSimParam->PRFOn()) {
        // The distance of the electron to the center of the pad in units of pad width
        double dist = (colOffset - 0.5 * padPlane->getColSize(colE)) / padPlane->getColSize(colE);
        // ********************************************************************************
        // This is a fixed parametrization, i.e. not dependent on calibration values !
        // ********************************************************************************
        if (!(mPRF->getPRF(signal, dist, layer, padSignal))) {
          continue;
        }
      } else {
        padSignal[0] = 0;
        padSignal[1] = signal;
        padSignal[2] = 0;
      }

      // The time bin (always positive), with t0 distortion
      double timeBinIdeal = driftTime * samplingRate + t0;
      // Protection
      if (abs(timeBinIdeal) > 2 * nTimeTotal) {
        timeBinIdeal = 2 * nTimeTotal;
      }
      int timeBinTruncated = ((int)timeBinIdeal);
      // The distance of the position to the middle of the timebin
      double timeOffset = ((float)timeBinTruncated + 0.5 - timeBinIdeal) / samplingRate;

      // Sample the time response inside the drift region + additional time bins before and after.
      // The sampling is done always in the middle of the time bin
      const int firstTimeBin = TMath::Max(timeBinTruncated, 0);
      const int lastTimeBin = TMath::Min(timeBinTruncated + timeBinTRFend, nTimeTotal);
      for (int iTimeBin = firstTimeBin; iTimeBin < lastTimeBin; ++iTimeBin) {
        // Apply the time response
        double timeResponse = 1;
        double crossTalk = 0;
        const double t = (iTimeBin - timeBinTruncated) / samplingRate + timeOffset;
        if (mSimParam->TRFOn()) {
          timeResponse = mSimParam->TimeResponse(t);
        }
        if (mSimParam->CTOn()) {
          crossTalk = mSimParam->CrossTalk(t);
        }
        signalOld[0] = 0;
        signalOld[1] = 0;
        signalOld[2] = 0;
        for (int iPad = 0; iPad < kNpad; iPad++) {
          int colPos = colE + iPad - 1;
          if (colPos < 0) {
            continue;
          }
          if (colPos >= nColMax) {
            break;
          }
          // Add the signals
          // signalOld[iPad] = arraySignal.getData(rowE, colPos, iTimeBin); // TO BE ADDED LATER REQUIRES ARRAYSIGNALS
          if (colPos != colE) {
            // Cross talk added to non-central pads
            signalOld[iPad] += padSignal[iPad] * (timeResponse + crossTalk);
          } else {
            // Without cross talk at central pad
            signalOld[iPad] += padSignal[iPad] * timeResponse;
          }
          // arraySignal.setData(rowE, colPos, iTimeBin, signalOld[iPad]); // TO BE ADDED LATER REQUIRES ARRAYSIGNALS
          // ******* TO BE ADDED LATER REQUIRES ARRAYSIGNALS
        } // Loop: pads
      }   // Loop: time bins
    }     // end of loop over electrons
  }       // end of loop over hits
  return true;
}

bool Digitizer::convertSignalsToDigits(const int det, int& arraySignal)
{
  //
  // Converstion of signals to digits
  //

  if (mSDigits) {
    // Convert the signal array to s-digits
    if (!convertSignalsToSDigits(det, arraySignal)) {
      return false;
    }
  } else {
    // Convert the signal array to digits
    if (!convertSignalsToADC(det, arraySignal)) {
      return false;
    }
    // Run digital processing for digits
    // RunDigitalProcessing(det);
  }
  // Compress the arrays
  // CompressOutputArrays(det);
  return true;
}

bool Digitizer::convertSignalsToSDigits(const int det, int& arraySignal)
{
  //
  // Convert signals to S-digits
  //

  return true;
}

bool Digitizer::convertSignalsToADC(const int det, int& signals)
{
  //
  // Converts the sampled electron signals to ADC values for a given chamber
  //

  // Converts number of electrons to fC
  constexpr double kEl2fC = 1.602e-19 * 1.0e15;

  // Coupling factor
  double coupling = mSimParam->GetPadCoupling() * mSimParam->GetTimeCoupling();
  // Electronics conversion factor
  double convert = kEl2fC * mSimParam->GetChipGain();
  // ADC conversion factor
  double adcConvert = mSimParam->GetADCoutRange() / mSimParam->GetADCinRange();
  // The electronics baseline in mV
  double baseline = mSimParam->GetADCbaseline() / adcConvert;
  // The electronics baseline in electrons
  double baselineEl = baseline / convert;

  int row = 0;
  int col = 0;
  int time = 0;

  int nRowMax = mGeo->getPadPlane(det)->getNrows();
  int nColMax = mGeo->getPadPlane(det)->getNcols();
  int nTimeTotal = kTimeBins; // fDigitsManager->GetDigitsParam()->GetNTimeBins(det);
  // if (fSDigitsManager->GetDigitsParam()->GetNTimeBins(det)) {
  //   nTimeTotal = fSDigitsManager->GetDigitsParam()->GetNTimeBins(det);
  // } else {
  //   LOG(FATAL) << "Could not get number of time bins";
  //   return false;
  // }

  // AliTRDarrayADC* digits = 0x0;
  int digits = 0;

  if (!signals) {
    LOG(FATAL) << "Signals array for detector " << det << " does not exis";
    return false;
  }

  // Get the mCalib objects
  // CalDet* calGainFactorDet = mCalib->GetGainFactorDet();
  // CalRoc* calGainFactorROC = mCalib->GetGainFactorROC(det);
  // calGainFactorDetValue = calGainFactorDet->GetValue(det);
  float calGainFactorDetValue = 0.47; // +/- 0.06 // Defaults value  from OCDB (AliRoot DrawTrending macro) for 5 TeV pp - 27 runs from LHC15n

  // Create the digits for this chamber
  for (row = 0; row < nRowMax; row++) {
    for (col = 0; col < nColMax; col++) {

      // halfchamber masking
      int iMcm = (int)(col / 18);               // current group of 18 col pads
      int halfchamberside = (iMcm > 3 ? 1 : 0); // 0=Aside, 1=Bside
      // Halfchambers that are switched off, masked by mCalib
      // if (mCalib->IsHalfChamberNoData(det, halfchamberside))
      //   continue;

      // Check whether pad is masked
      // Bridged pads are not considered yet!!!
      // if (mCalib->IsPadMasked(det, col, row) ||
      //     mCalib->IsPadNotConnected(det, col, row)) {
      //   continue;
      // }

      // The gain factors
      float padgain = calGainFactorDetValue; // * calGainFactorROC->GetValue(col, row); // PLEASE FIX ME when CCDB is ready
      if (padgain <= 0) {
        const auto msg = Form("Not a valid gain %f, %d %d %d", padgain, det, col, row);
        LOG(FATAL) << msg;
      }

      for (time = 0; time < nTimeTotal; time++) {
        // Get the signal amplitude
        float signalAmp = 99; //signals->GetData(row, col, time);
        // Pad and time coupling
        signalAmp *= coupling;
        // Gain factors
        signalAmp *= padgain;
        // Add the noise, starting from minus ADC baseline in electrons
        signalAmp = TMath::Max((double)gRandom->Gaus(signalAmp, mSimParam->GetNoise()), -baselineEl);
        // Convert to mV
        signalAmp *= convert;
        // Add ADC baseline in mV
        signalAmp += baseline;
        // Convert to ADC counts. Set the overflow-bit fADCoutRange if the
        // signal is larger than fADCinRange
        short adc = 0;
        if (signalAmp >= mSimParam->GetADCinRange()) {
          adc = ((short)mSimParam->GetADCoutRange());
        } else {
          adc = TMath::Nint(signalAmp * adcConvert);
        }
        // Saving all digits
        // digits->SetData(row, col, time, adc);
      } // for: time
    }   // for: col
  }     // for: row
  return true;
}

bool Digitizer::diffusion(float vdrift, double absdriftlength, double exbvalue, double& lRow, double& lCol, double& lTime)
{
  //
  // Applies the diffusion smearing to the position of a single electron.
  // Depends on absolute drift length.
  //
  float diffL = 0.0;
  float diffT = 0.0;
  if (mCommonParam->GetDiffCoeff(diffL, diffT, vdrift)) {
    float driftSqrt = TMath::Sqrt(absdriftlength);
    float sigmaT = driftSqrt * diffT;
    float sigmaL = driftSqrt * diffL;
    lRow = gRandom->Gaus(lRow, sigmaT);
    if (mCommonParam->ExBOn()) {
      lCol = gRandom->Gaus(lCol, sigmaT * 1.0 / (1.0 + exbvalue * exbvalue));
      lTime = gRandom->Gaus(lTime, sigmaL * 1.0 / (1.0 + exbvalue * exbvalue));
    } else {
      lCol = gRandom->Gaus(lCol, sigmaT);
      lTime = gRandom->Gaus(lTime, sigmaL);
    }
    return true;
  } else {
    return false;
  }
}
