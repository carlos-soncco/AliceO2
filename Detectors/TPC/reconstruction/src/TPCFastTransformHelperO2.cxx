// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// \file TPCFastTransformHelperO2.cxx
/// \author Sergey Gorbunov

#include "TPCReconstruction/TPCFastTransformHelperO2.h"

#include "TPCBase/Mapper.h"
#include "TPCBase/PadRegionInfo.h"
#include "TPCBase/ParameterDetector.h"
#include "TPCBase/ParameterElectronics.h"
#include "TPCBase/ParameterGas.h"
#include "TPCBase/Sector.h"
#include "DataFormatsTPC/Defs.h"
#include "TPCFastTransform.h"
#include "Riostream.h"
#include "FairLogger.h"

using namespace o2::gpu;

namespace o2
{
namespace tpc
{

TPCFastTransformHelperO2* TPCFastTransformHelperO2::sInstance = nullptr;

TPCFastTransformHelperO2* TPCFastTransformHelperO2::instance()
{
  // returns TPCFastTransformHelperO2 instance (singleton)
  if (!sInstance) {
    sInstance = new TPCFastTransformHelperO2();
    sInstance->init();
  }
  return sInstance;
}

void TPCFastTransformHelperO2::init()
{
  // do nothing at the moment
  mIsInitialized = 1;
}

std::unique_ptr<TPCFastTransform> TPCFastTransformHelperO2::create(Long_t TimeStamp)
{
  /// initializes TPCFastTransform object

  std::unique_ptr<TPCFastTransform> fastTransformPtr(new TPCFastTransform);

  TPCFastTransform& fastTransform = *fastTransformPtr;

  // tell the transformation to apply the space charge distortions

  // FIXME: this is the awful initialization flag
  constexpr int fastTransformFlag = 1;
  fastTransform.setApplyDistortionFlag(fastTransformFlag);

  if (!mIsInitialized) {
    init();
  }

  auto& detParam = ParameterDetector::Instance();
  const static ParameterGas& gasParam = ParameterGas::Instance();
  const static ParameterElectronics& elParam = ParameterElectronics::Instance();

  const double vDrift = (elParam.ZbinWidth * gasParam.DriftV); // cm/timebin

  // find last calibrated time bin
  const double lastTimeBin = detParam.TPClength / vDrift + 1;

  const Mapper& mapper = Mapper::instance();

  const int nRows = mapper.getNumberOfRows();

  fastTransform.startConstruction(nRows);

  TPCDistortionIRS& distortion = fastTransform.getDistortionNonConst();

  distortion.startConstruction(nRows, 1);

  float tpcZlengthSideA = detParam.TPClength;
  float tpcZlengthSideC = detParam.TPClength;

  fastTransform.setTPCgeometry(tpcZlengthSideA, tpcZlengthSideC);
  distortion.setTPCgeometry(tpcZlengthSideA, tpcZlengthSideC);

  for (int iRow = 0; iRow < fastTransform.getNumberOfRows(); iRow++) {
    Sector sector = 0;
    int regionNumber = 0;
    while (iRow >= mapper.getGlobalRowOffsetRegion(regionNumber) + mapper.getNumberOfRowsRegion(regionNumber))
      regionNumber++;

    const PadRegionInfo& region = mapper.getPadRegionInfo(regionNumber);

    int nPads = mapper.getNumberOfPadsInRowSector(iRow);
    float padWidth = region.getPadWidth();

    const GlobalPadNumber pad = mapper.globalPadNumber(PadPos(iRow, nPads / 2));
    const PadCentre& padCentre = mapper.padCentre(pad);
    float xRow = padCentre.X();

    fastTransform.setTPCrow(iRow, xRow, nPads, padWidth);
    distortion.setTPCrow(iRow, xRow, nPads, padWidth, 0);
  }

  // set some initial values, will be reinitialised later int updateCalibration()
  const double t0 = 0.;
  const double vdCorrY = 0.;
  const double ldCorr = 0.;
  const double tpcAlignmentZ = 0.;
  const double tofCorr = 0.;
  const double primVtxZ = 0.;
  const double initTimeStamp = -1;
  fastTransform.setCalibration(initTimeStamp, t0, vDrift, vdCorrY, ldCorr, tofCorr, primVtxZ, tpcAlignmentZ);

  IrregularSpline2D3D spline;
  {
    int nKnotsU = 15;
    int nAxisTicksU = mapper.getNumberOfPadsInRowSector(10);
    int nKnotsV = 20;
    int nAxisTicksV = lastTimeBin + 1;
    float knotsU[nKnotsU];
    float knotsV[nKnotsV];
    for (int i = 0; i < nKnotsU; i++)
      knotsU[i] = 1. / (nKnotsU - 1) * i;
    for (int i = 0; i < nKnotsV; i++)
      knotsV[i] = 1. / (nKnotsV - 1) * i;

    // TODO: adjust the grid

    double d1 = 0.6;
    double d2 = 0.9 - d1;
    double d3 = 1. - d2 - d1;

    for (int i = 0; i < 5; i++) { // 5 bins in first 6% of drift
      knotsV[i] = i / 4. * d1;
    }
    for (int i = 0; i < 10; i++) { // 10 bins for 6% <-> 90%
      knotsV[4 + i] = d1 + i / 9. * d2;
    }
    for (int i = 0; i < 5; i++) { // 5 bins for last 90% <-> 100%
      knotsV[13 + i] = d1 + d2 + i / 4. * d3;
    }

    spline.construct(nKnotsU, knotsU, nAxisTicksU,
                     nKnotsV, knotsV, nAxisTicksV);
  }
  distortion.setApproximationScenario(0, spline);
  distortion.finishConstruction();
  fastTransform.finishConstruction();

  // check if calculated pad geometry is consistent with the map
  testGeometry(fastTransform);

  updateCalibration(fastTransform, TimeStamp);

  return std::move(fastTransformPtr);
}

int TPCFastTransformHelperO2::updateCalibration(TPCFastTransform& fastTransform, Long_t TimeStamp)
{
  // Update the calibration with the new time stamp

  if (!mIsInitialized) {
    init();
  }

  Long_t lastTS = fastTransform.getTimeStamp();

  // deinitialize

  fastTransform.setTimeStamp(-1);

  if (TimeStamp < 0) {
    return 0;
  }

  // search for the calibration database ...

  auto& detParam = ParameterDetector::Instance();
  const static ParameterGas& gasParam = ParameterGas::Instance();
  const static ParameterElectronics& elParam = ParameterElectronics::Instance();

  // calibration found, set the initialized status back

  fastTransform.setTimeStamp(lastTS);

  // less than 60 seconds from the previois time stamp, don't do anything

  if (lastTS >= 0 && TMath::Abs(lastTS - TimeStamp) < 60) {
    return 0;
  }

  // start the initialization

  fastTransform.setTimeStamp(TimeStamp);

  // find last calibrated time bin

  const double vDrift = elParam.ZbinWidth * gasParam.DriftV; // cm/timebin

  //mLastTimeBin = detParam.getTPClength() / vDrift  + 1;

  // fast transform formula:
  // L = (t-t0)*(mVdrift + mVdriftCorrY*yLab ) + mLdriftCorr
  // Z = Z(L) +  tpcAlignmentZ
  // spline distortions for xyz
  // Time-of-flight correction: ldrift += dist-to-vtx*tofCorr

  const double t0 = elParam.PeakingTime / elParam.ZbinWidth;

  const double vdCorrY = 0.;
  const double ldCorr = 0.;
  const double tpcAlignmentZ = 0.;

  const double tofCorr = 0.;
  const double primVtxZ = 0.;

  fastTransform.setCalibration(TimeStamp, t0, vDrift, vdCorrY, ldCorr, tofCorr, primVtxZ, tpcAlignmentZ);

  // now calculate distortion map: dx,du,dv = ( origTransform() -> x,u,v) - fastTransformNominal:x,u,v

  TPCDistortionIRS& distortion = fastTransform.getDistortionNonConst();

  // for the future: switch TOF correction off for a while

  for (int slice = 0; slice < distortion.getNumberOfSlices(); slice++) {
    const TPCFastTransform::SliceInfo& sliceInfo = fastTransform.getSliceInfo(slice);

    for (int row = 0; row < distortion.getNumberOfRows(); row++) {
      const TPCFastTransform::RowInfo& rowInfo = fastTransform.getRowInfo(row);

      const IrregularSpline2D3D& spline = distortion.getSpline(slice, row);
      float* data = distortion.getSplineDataNonConst(slice, row);

      for (int knot = 0; knot < spline.getNumberOfKnots(); knot++) {

        if (!mSpaceChargeCorrection) {
          data[3 * knot + 0] = 0.f;
          data[3 * knot + 1] = 0.f;
          data[3 * knot + 2] = 0.f;
          continue;
        }

        // x cordinate of the knot
        float x = rowInfo.x;

        // spline (su,sv) cordinates of the knot.  (su,sv) are in (0,1)x(0,1) area

        float su = 0, sv = 0;
        spline.getKnotUV(knot, su, sv);

        // x, u, v cordinates of the knot (local cartesian coord. of slice towards central electrode )
        float u = 0, v = 0;
        distortion.convSUVtoUV(slice, row, su, sv, u, v);

        // nominal x,y,z coordinates of the knot (without distortions and time-of-flight correction)
        float y = 0, z = 0;
        fastTransform.convUVtoYZ(slice, row, x, u, v, y, z);

        // global coordinates of the knot
        // TODO: add a method to the fast transform
        //fastTransform.convLocalToGlobal( slice, x, y, z, x, y, z );

        float gx = x * sliceInfo.cosAlpha - y * sliceInfo.sinAlpha;
        float gy = x * sliceInfo.sinAlpha + y * sliceInfo.cosAlpha;
        float gz = z;

        float gx1 = gx, gy1 = gy, gz1 = gz;

        {
          double xyz[3] = { gx, gy, gz };
          double dxyz[3] = { 0., 0., 0. };
          mSpaceChargeCorrection(xyz, dxyz);
          gx1 += dxyz[0];
          gy1 += dxyz[1];
          gz1 += dxyz[2];
        }

        // corrections in the local coordinates
        // TODO: add a method to the fast transform
        // fastTransform.convGlobalToLocal( slice, dx, dy, dz, dx, dy, dz );

        float x1 = gx1 * sliceInfo.cosAlpha + gy1 * sliceInfo.sinAlpha;
        float y1 = -gx1 * sliceInfo.sinAlpha + gy1 * sliceInfo.cosAlpha;
        float z1 = gz1;

        // distortion corrections in u,v
        float u1 = 0, v1 = 0;
        fastTransform.convYZtoUV(slice, row, x1, y1, z1, u1, v1);

        data[3 * knot + 0] = x1 - x;
        data[3 * knot + 1] = u1 - u;
        data[3 * knot + 2] = v1 - v;

      } // knots
      spline.correctEdges(data);
    } // row
  }   // slice

  // for the future: set back the time-of-flight correction

  return 0;
}

void TPCFastTransformHelperO2::testGeometry(const TPCFastTransform& fastTransform) const
{
  const Mapper& mapper = Mapper::instance();

  if (fastTransform.getNumberOfSlices() != Sector::MAXSECTOR) {
    LOG(FATAL) << "Wrong number of sectors :" << fastTransform.getNumberOfSlices() << " instead of " << Sector::MAXSECTOR << std::endl;
  }

  if (fastTransform.getNumberOfRows() != mapper.getNumberOfRows()) {
    LOG(FATAL) << "Wrong number of rows :" << fastTransform.getNumberOfRows() << " instead of " << mapper.getNumberOfRows() << std::endl;
  }

  double maxDx = 0, maxDy = 0;

  for (int row = 0; row < fastTransform.getNumberOfRows(); row++) {

    const int nPads = fastTransform.getRowInfo(row).maxPad + 1;

    if (nPads != mapper.getNumberOfPadsInRowSector(row)) {
      LOG(FATAL) << "Wrong number of pads :" << nPads << " instead of " << mapper.getNumberOfPadsInRowSector(row) << std::endl;
    }

    const double x = fastTransform.getRowInfo(row).x;

    // check if calculated pad positions are equal to the real ones

    for (int pad = 0; pad < nPads; pad++) {
      const GlobalPadNumber p = mapper.globalPadNumber(PadPos(row, pad));
      const PadCentre& c = mapper.padCentre(p);
      float u = 0, v = 0;
      int err = fastTransform.convPadTimeToUV(0, row, pad, 10., u, v, 0.);
      if (err != 0) {
        LOG(FATAL) << "Can not transform a cluster: row " << row << " pad " << pad << " time 10. : error " << err << std::endl;
      }

      const double dx = x - c.X();
      const double dy = u - (-c.Y()); // diferent sign convention for Y coordinate in the map

      if (fabs(dx) >= 1.e-6 || fabs(dy) >= 1.e-5) {
        LOG(WARNING) << "wrong calculated pad position:"
                     << " row " << row << " pad " << pad << " x calc " << x << " x in map " << c.X() << " dx " << (x - c.X())
                     << " y calc " << u << " y in map " << -c.Y() << " dy " << dy << std::endl;
      }
      if (fabs(maxDx) < fabs(dx)) {
        maxDx = dx;
      }
      if (fabs(maxDy) < fabs(dy)) {
        maxDy = dy;
      }
    }
  }

  if (fabs(maxDx) >= 1.e-4 || fabs(maxDy) >= 1.e-4) {
    LOG(FATAL) << "wrong calculated pad position:"
               << " max Dx " << maxDx << " max Dy " << maxDy << std::endl;
  }
}
} // namespace tpc
} // namespace o2
