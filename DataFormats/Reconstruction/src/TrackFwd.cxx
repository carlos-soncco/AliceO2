// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "ReconstructionDataFormats/TrackFwd.h"
#include "Math/MatrixFunctions.h"

namespace o2
{
namespace track
{
using namespace std;

//_________________________________________________________________________
TrackParCovFwd::TrackParCovFwd(const Double_t z, const SMatrix5 parameters, const SMatrix55 covariances, const Double_t chi2)
{
  setZ(z);
  setParameters(parameters);
  setCovariances(covariances);
  setTrackChi2(chi2);
}

//__________________________________________________________________________
void TrackParFwd::propagateParamToZlinear(double zEnd)
{
  // Track parameters linearly extrapolated to the plane at "zEnd".

  if (getZ() == zEnd) {
    return; // nothing to be done if same z
  }

  // Compute track parameters
  auto dZ = (zEnd - getZ());
  auto x0 = getX();
  auto y0 = getY();
  auto phi0 = getPhi();
  double cosphi0, sinphi0;
  o2::utils::sincos(phi0, sinphi0, cosphi0);
  auto invtanl0 = 1.0 / getTanl();
  auto n = dZ * invtanl0;
  auto x = x0 + n * cosphi0;
  auto y = y0 + n * sinphi0;
  setX(x);
  setY(y);
  setZ(zEnd);
}

//__________________________________________________________________________
void TrackParCovFwd::propagateToZlinear(double zEnd)
{
  // Track parameters and their covariances linearly extrapolated to the plane at "zEnd".

  // Calculate the jacobian related to the track parameters extrapolated to "zEnd"
  auto dZ = (zEnd - getZ());
  auto x0 = getX();
  auto y0 = getY();
  auto phi0 = getPhi();
  auto tanl0 = getTanl();
  auto invtanl0 = 1.0 / tanl0;
  double cosphi0, sinphi0;
  o2::utils::sincos(phi0, sinphi0, cosphi0);
  auto n = dZ * invtanl0;
  auto m = n * invtanl0;

  // Extrapolate track parameters to "zEnd"
  mParameters(0) += n * cosphi0;
  mParameters(1) += n * sinphi0;
  setZ(zEnd);

  // Calculate Jacobian
  SMatrix55 jacob = ROOT::Math::SMatrixIdentity();
  jacob(0, 2) = -n * sinphi0;
  jacob(0, 3) = -m * cosphi0;
  jacob(1, 2) = n * cosphi0;
  jacob(1, 3) = -m * sinphi0;

  // Extrapolate track parameter covariances to "zEnd"
  setCovariances(ROOT::Math::Similarity(jacob, mCovariances));
}

//__________________________________________________________________________
void TrackParFwd::propagateParamToZquadratic(double zEnd, double zField)
{
  // Track parameters extrapolated to the plane at "zEnd" considering a helix

  if (getZ() == zEnd) {
    return; // nothing to be done if same z
  }

  // Compute track parameters
  auto dZ = (zEnd - getZ());
  auto phi0 = getPhi();
  double cosphi0, sinphi0;
  o2::utils::sincos(phi0, sinphi0, cosphi0);
  auto invtanl0 = 1.0 / getTanl();
  auto invqpt0 = getInvQPt();
  auto Hz = std::copysign(1, zField);
  auto k = TMath::Abs(o2::constants::math::B2C * zField);
  auto n = dZ * invtanl0;
  auto theta = -invqpt0 * dZ * k * invtanl0;

  mParameters(0) += n * cosphi0 - 0.5 * n * theta * Hz * sinphi0;
  mParameters(1) += n * sinphi0 + 0.5 * n * theta * Hz * cosphi0;
  mParameters(2) += Hz * theta;
  setZ(zEnd);
}

//__________________________________________________________________________
void TrackParCovFwd::propagateToZquadratic(double zEnd, double zField)
{
  // Extrapolate track parameters and covariances matrix to "zEnd"
  // using quadratic track model

  if (getZ() == zEnd) {
    return; // nothing to be done if same z
  }

  // Compute track parameters
  auto dZ = (zEnd - getZ());
  auto x0 = getX();
  auto y0 = getY();
  auto phi0 = getPhi();
  double cosphi0, sinphi0;
  o2::utils::sincos(phi0, sinphi0, cosphi0);
  auto invtanl0 = 1.0 / getTanl();
  auto invqpt0 = getInvQPt();
  auto Hz = std::copysign(1, zField);
  auto k = TMath::Abs(o2::constants::math::B2C * zField);
  auto n = dZ * invtanl0;
  auto m = n * invtanl0;
  auto theta = -invqpt0 * dZ * k * invtanl0;

  // Extrapolate track parameters to "zEnd"
  auto x = x0 + n * cosphi0 - 0.5 * n * theta * Hz * sinphi0;
  auto y = y0 + n * sinphi0 + 0.5 * n * theta * Hz * cosphi0;
  auto phi = phi0 + Hz * theta;
  setX(x);
  setY(y);
  setZ(zEnd);
  setPhi(phi);

  // Calculate Jacobian
  SMatrix55 jacob = ROOT::Math::SMatrixIdentity();
  jacob(0, 2) = -n * theta * 0.5 * Hz * cosphi0 - n * sinphi0;
  jacob(0, 3) = Hz * m * theta * sinphi0 - m * cosphi0;
  jacob(0, 4) = k * m * 0.5 * Hz * dZ * sinphi0;
  jacob(1, 2) = -n * theta * 0.5 * Hz * sinphi0 + n * cosphi0;
  jacob(1, 3) = -Hz * m * theta * cosphi0 - m * sinphi0;
  jacob(1, 4) = -k * m * 0.5 * Hz * dZ * cosphi0;
  jacob(2, 3) = -Hz * theta * invtanl0;
  jacob(2, 4) = -Hz * k * n;

  // Extrapolate track parameter covariances to "zEnd"
  setCovariances(ROOT::Math::Similarity(jacob, mCovariances));
}

//__________________________________________________________________________
void TrackParFwd::propagateParamToZhelix(double zEnd, double zField)
{
  // Track parameters extrapolated to the plane at "zEnd"
  // using helix track model

  if (getZ() == zEnd) {
    return; // nothing to be done if same z
  }

  // Compute track parameters
  auto dZ = (zEnd - getZ());
  auto x0 = getX();
  auto y0 = getY();
  auto phi0 = getPhi();
  auto tanl0 = getTanl();
  auto invtanl0 = 1.0 / tanl0;
  auto invqpt0 = getInvQPt();
  auto qpt0 = 1.0 / invqpt0;
  double cosphi0, sinphi0;
  o2::utils::sincos(phi0, sinphi0, cosphi0);
  auto k = TMath::Abs(o2::constants::math::B2C * zField);
  auto invk = 1.0 / k;
  auto theta = -invqpt0 * dZ * k * invtanl0;
  double costheta, sintheta;
  o2::utils::sincos(theta, sintheta, costheta);
  auto Hz = std::copysign(1, zField);
  auto Y = sinphi0 * qpt0 * invk;
  auto X = cosphi0 * qpt0 * invk;
  auto YC = Y * costheta;
  auto YS = Y * sintheta;
  auto XC = X * costheta;
  auto XS = X * sintheta;

  // Extrapolate track parameters to "zEnd"
  auto x = x0 + Hz * (Y - YC) - XS;
  auto y = y0 + Hz * (-X + XC) - YS;
  auto phi = phi0 + Hz * theta;
  setX(x);
  setY(y);
  setZ(zEnd);
  setPhi(phi);
}

//__________________________________________________________________________
void TrackParCovFwd::propagateToZhelix(double zEnd, double zField)
{
  // Extrapolate track parameters and covariances matrix to "zEnd"
  // using helix track model

  auto dZ = (zEnd - getZ());
  auto x0 = getX();
  auto y0 = getY();
  auto phi0 = getPhi();
  auto tanl0 = getTanl();
  auto invtanl0 = 1.0 / tanl0;
  auto invqpt0 = getInvQPt();
  auto qpt0 = 1.0 / invqpt0;
  double cosphi0, sinphi0;
  o2::utils::sincos(phi0, sinphi0, cosphi0);
  auto k = TMath::Abs(o2::constants::math::B2C * zField);
  auto invk = 1.0 / k;
  auto theta = -invqpt0 * dZ * k * invtanl0;
  double costheta, sintheta;
  o2::utils::sincos(theta, sintheta, costheta);
  auto Hz = std::copysign(1, zField);
  auto L = qpt0 * qpt0 * invk;
  auto N = dZ * invtanl0 * qpt0;
  auto O = sintheta * cosphi0;
  auto P = sinphi0 * costheta;
  auto R = sinphi0 * sintheta;
  auto S = cosphi0 * costheta;
  auto Y = sinphi0 * qpt0 * invk;
  auto X = cosphi0 * qpt0 * invk;
  auto YC = Y * costheta;
  auto YS = Y * sintheta;
  auto XC = X * costheta;
  auto XS = X * sintheta;
  auto T = qpt0 * costheta;
  auto U = qpt0 * sintheta;
  auto V = qpt0;
  auto n = dZ * invtanl0;
  auto m = n * invtanl0;

  // Extrapolate track parameters to "zEnd"
  auto x = x0 + Hz * (Y - YC) - XS;
  auto y = y0 + Hz * (-X + XC) - YS;
  auto phi = phi0 + Hz * theta;
  setX(x);
  setY(y);
  setZ(zEnd);
  setPhi(phi);

  // Calculate Jacobian
  SMatrix55 jacob = ROOT::Math::SMatrixIdentity();
  jacob(0, 2) = Hz * X - Hz * XC + YS;
  jacob(0, 3) = Hz * R * m - S * m;
  jacob(0, 4) = -Hz * N * R + Hz * T * Y - Hz * V * Y + N * S + U * X;
  jacob(1, 2) = Hz * Y - Hz * YC - XS;
  jacob(1, 3) = -Hz * O * m - P * m;
  jacob(1, 4) = Hz * N * O - Hz * T * X + Hz * V * X + N * P + U * Y;
  jacob(2, 3) = -Hz * theta * invtanl0;
  jacob(2, 4) = -Hz * k * n;

  // Extrapolate track parameter covariances to "zEnd"
  setCovariances(ROOT::Math::Similarity(jacob, mCovariances));
}

//__________________________________________________________________________
void TrackParCovFwd::addMCSEffect(double dZ, double x_over_X0)
{
  /// Add multiple Coulomb scattering effects to the track parameter covariances.
  ///  * if (dZ > 0): MCS effects are evaluated with a linear propagation model.
  ///  * if (dZ <= 0): only angular MCS effects are evaluated as if dZ = 0.
  ///  * x_over_X0 is the fraction of the radiation lenght (x/X0).
  ///  * No energy loss correction.
  ///  * All scattering evaluated at the position of the first cluster.

  auto phi0 = getPhi();
  auto tanl0 = getTanl();
  auto invtanl0 = 1.0 / tanl0;
  auto invqpt0 = getInvQPt();

  double cosphi0, sinphi0;
  o2::utils::sincos(phi0, sinphi0, cosphi0);

  auto csclambda = TMath::Abs(TMath::Sqrt(1 + tanl0 * tanl0) * invtanl0);
  auto pathLengthOverX0 = x_over_X0 * csclambda; //

  // Angular dispersion square of the track (variance) in a plane perpendicular to the trajectory
  auto sigmathetasq = 0.0136 * getInverseMomentum() * (1 + 0.038 * TMath::Log(pathLengthOverX0));
  sigmathetasq *= sigmathetasq * pathLengthOverX0;

  // Get covariance matrix
  SMatrix55 newParamCov(getCovariances());

  if (dZ > 0) {
    auto A = tanl0 * tanl0 + 1;
    auto B = dZ * cosphi0 * invtanl0;
    auto C = dZ * sinphi0 * invtanl0;
    auto D = A * B * invtanl0;
    auto E = -A * C * invtanl0;
    auto F = -C - D;
    auto G = B + E;
    auto H = -invqpt0 * tanl0;

    newParamCov(0, 0) += sigmathetasq * F * F;

    newParamCov(0, 1) += sigmathetasq * F * G;

    newParamCov(1, 1) += sigmathetasq * G * G;

    newParamCov(2, 0) += sigmathetasq * F;

    newParamCov(2, 1) += sigmathetasq * G;

    newParamCov(2, 2) += sigmathetasq;

    newParamCov(3, 0) += sigmathetasq * A * F;

    newParamCov(3, 1) += sigmathetasq * A * G;

    newParamCov(3, 2) += sigmathetasq * A;

    newParamCov(3, 3) += sigmathetasq * A * A;

    newParamCov(4, 0) += sigmathetasq * F * H;

    newParamCov(4, 1) += sigmathetasq * G * H;

    newParamCov(4, 2) += sigmathetasq * H;

    newParamCov(4, 3) += sigmathetasq * A * H;

    newParamCov(4, 4) += sigmathetasq * tanl0 * tanl0 * invqpt0 * invqpt0;
  } else {

    auto A = tanl0 * tanl0 + 1;
    auto H = -invqpt0 * tanl0;

    newParamCov(2, 2) += sigmathetasq;

    newParamCov(3, 2) += sigmathetasq * A;

    newParamCov(3, 3) += sigmathetasq * A * A;

    newParamCov(4, 2) += sigmathetasq * H;

    newParamCov(4, 3) += sigmathetasq * A * H;

    newParamCov(4, 4) += sigmathetasq * tanl0 * tanl0 * invqpt0 * invqpt0;
  }

  // Set new covariances
  setCovariances(newParamCov);
}

} // namespace track
} // namespace o2
