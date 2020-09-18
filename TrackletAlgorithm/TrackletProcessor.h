#ifndef TrackletAlgorithm_TrackletProcessor_h
#define TrackletAlgorithm_TrackletProcessor_h

#include <cmath>

#include "AllStubMemory.h"
#include "AllStubInnerMemory.h"
#include "TrackletParameterMemory.h"
#include "TrackletProjectionMemory.h"
#include "VMStubTEOuterMemoryCM.h"
#include "TEBuffer.h"
#include "TrackletEngineUnit.h"

namespace TC {
////////////////////////////////////////////////////////////////////////////////
// Typedefs, enums, and constants needed by TrackletCalculator.
////////////////////////////////////////////////////////////////////////////////
  namespace Types {
    typedef ap_uint<2> nASMem;

    typedef ap_int<13> rmean;
    typedef ap_int<14> zmean;
    typedef ap_int<15> rinv;
    typedef ap_int<11> z0;
    typedef ap_uint<20> phiL;
    typedef ap_int<15> zL;
    typedef ap_int<11> der_phiL;
    typedef ap_int<10> der_zL;
    typedef ap_uint<16> phiD;
    typedef ap_int<14> rD;
    typedef ap_int<10> der_phiD;
    typedef ap_int<10> der_rD;
    typedef ap_uint<1> flag;
  }

  enum seed {UNDEF_SEED, L1L2 = 0, L2L3 = 1, L3L4 = 2, L5L6 = 3, D1D2 = 4, D3D4 = 5, L1D1 = 6, L2D1 = 7};
  enum itc {UNDEF_ITC, A = 0, B = 1, C = 2, D = 3, E = 4, F = 5, G = 6, H = 7, I = 8, J = 9, K = 10, L = 11, M = 12, N = 13, O = 14};
  enum layer {L1 = 0, L2 = 1, L3 = 2, L4 = 3, L5 = 4, L6 = 5};
  enum disk {D1 = 0, D2 = 1, D3 = 2, D4 = 3, D5 = 4};
  enum projout_index_barrel_ps {L1PHIA = 0, L1PHIB = 1, L1PHIC = 2, L1PHID = 3, L1PHIE = 4, L1PHIF = 5, L1PHIG = 6, L1PHIH = 7, L2PHIA = 8, L2PHIB = 9, L2PHIC = 10, L2PHID = 11, L3PHIA = 12, L3PHIB = 13, L3PHIC = 14, L3PHID = 15, N_PROJOUT_BARRELPS = 16};
  enum projout_index_barrel_2s {L4PHIA = 0, L4PHIB = 1, L4PHIC = 2, L4PHID = 3, L5PHIA = 4, L5PHIB = 5, L5PHIC = 6, L5PHID = 7, L6PHIA = 8, L6PHIB = 9, L6PHIC = 10, L6PHID = 11, N_PROJOUT_BARREL2S = 12};
  enum projout_index_disk      {D1PHIA = 0, D1PHIB = 1, D1PHIC = 2, D1PHID = 3, D2PHIA = 4, D2PHIB = 5, D2PHIC = 6, D2PHID = 7, D3PHIA = 8, D3PHIB = 9, D3PHIC = 10, D3PHID = 11, D4PHIA = 12, D4PHIB = 13, D4PHIC = 14, D4PHID = 15, N_PROJOUT_DISK = 16};

  static const uint8_t nproj_L1 = L1PHIH - L1PHIA + 1;
  static const uint8_t nproj_L2 = L2PHID - L2PHIA + 1;
  static const uint8_t nproj_L3 = L3PHID - L3PHIA + 1;
  static const uint8_t nproj_L4 = L4PHID - L4PHIA + 1;
  static const uint8_t nproj_L5 = L5PHID - L5PHIA + 1;
  static const uint8_t nproj_L6 = L6PHID - L6PHIA + 1;
  static const uint8_t nproj_D1 = D1PHID - D1PHIA + 1;
  static const uint8_t nproj_D2 = D2PHID - D2PHIA + 1;
  static const uint8_t nproj_D3 = D3PHID - D3PHIA + 1;
  static const uint8_t nproj_D4 = D4PHID - D4PHIA + 1;

  static const uint8_t shift_L1 = 0;
  static const uint8_t shift_L2 = shift_L1 + nproj_L1;
  static const uint8_t shift_L3 = shift_L2 + nproj_L2;
  static const uint8_t shift_L4 = shift_L3 + nproj_L3;
  static const uint8_t shift_L5 = shift_L4 + nproj_L4;
  static const uint8_t shift_L6 = shift_L5 + nproj_L5;

  static const uint8_t shift_D1 = 0;
  static const uint8_t shift_D2 = shift_D1 + nproj_D1;
  static const uint8_t shift_D3 = shift_D2 + nproj_D2;
  static const uint8_t shift_D4 = shift_D3 + nproj_D3;

  static const uint32_t mask_L1 = 0xFF << shift_L1;
  static const uint32_t mask_L2 = 0xF << shift_L2;
  static const uint32_t mask_L3 = 0xF << shift_L3;
  static const uint32_t mask_L4 = 0xF << shift_L4;
  static const uint32_t mask_L5 = 0xF << shift_L5;
  static const uint32_t mask_L6 = 0xF << shift_L6;
  static const uint32_t mask_D1 = 0xF << shift_D1;
  static const uint32_t mask_D2 = 0xF << shift_D2;
  static const uint32_t mask_D3 = 0xF << shift_D3;
  static const uint32_t mask_D4 = 0xF << shift_D4;

// the 1.0e-1 is a fudge factor needed to get the floating point truncation
// right
  static const float ptcut = 1.91;
  static const ap_uint<13> rinvcut = 0.01 * 0.3 * 3.8 / ptcut / krinv + 1.0e-1;
  static const ap_uint<9> z0cut_L1L2 = 15.0 / kz0 + 1.0e-1;
  static const ap_uint<9> z0cut = 1.5 * 15.0 / kz0 + 1.0e-1;
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Functions that are defined in TrackletCalculator_calculate_LXLY.h,
// TrackletProcessor.cc, and the bottom of this file.
////////////////////////////////////////////////////////////////////////////////
  template<seed Seed, regionType InnerRegion, regionType OuterRegion>
  void calculate_LXLY (
    const typename AllStub<InnerRegion>::ASR r1_input,
    const typename AllStub<InnerRegion>::ASPHI phi1_input,
    const typename AllStub<InnerRegion>::ASZ z1_input,
    const typename AllStub<OuterRegion>::ASR r2_input,
    const typename AllStub<OuterRegion>::ASPHI phi2_input,
    const typename AllStub<OuterRegion>::ASZ z2_input,
    const Types::rmean r1mean_input,
    const Types::rmean r2mean_input,
    const Types::rmean rproj0_input,
    const Types::rmean rproj1_input,
    const Types::rmean rproj2_input,
    const Types::rmean rproj3_input,
    const Types::zmean zproj0_input,
    const Types::zmean zproj1_input,
    const Types::zmean zproj2_input,
    const Types::zmean zproj3_input,

    Types::rinv * const rinv_output,
    TrackletParameters::PHI0PAR * const phi0_output,
    TrackletParameters::TPAR * const t_output,
    Types::z0 * const z0_output,
    Types::phiL * const phiL_0_output,
    Types::phiL * const phiL_1_output,
    Types::phiL * const phiL_2_output,
    Types::phiL * const phiL_3_output,
    Types::zL * const zL_0_output,
    Types::zL * const zL_1_output,
    Types::zL * const zL_2_output,
    Types::zL * const zL_3_output,
    Types::der_phiL * const der_phiL_output,
    Types::der_zL * const der_zL_output,
    Types::phiD * const phiD_0_output,
    Types::phiD * const phiD_1_output,
    Types::phiD * const phiD_2_output,
    Types::phiD * const phiD_3_output,
    Types::rD * const rD_0_output,
    Types::rD * const rD_1_output,
    Types::rD * const rD_2_output,
    Types::rD * const rD_3_output,
    Types::der_phiD * const der_phiD_output,
    Types::der_rD * const der_rD_output
  );

  template<seed Seed, regionType InnerRegion, regionType OuterRegion> bool barrelSeeding(const AllStub<InnerRegion> &innerStub, const AllStub<OuterRegion> &outerStub, Types::rinv * const rinv, TrackletParameters::PHI0PAR * const phi0, Types::z0 * const z0, TrackletParameters::TPAR * const t, Types::phiL phiL[4], Types::zL zL[4], Types::der_phiL * const der_phiL, Types::der_zL * const der_zL, Types::flag valid_proj[4], Types::phiD phiD[4], Types::rD rD[4], Types::der_phiD * const der_phiD, Types::der_rD * const der_rD, Types::flag valid_proj_disk[4]);

  template<seed Seed, itc iTC> const TrackletProjection<BARRELPS>::TProjTCID ID();

  template<regionType TProjType, uint8_t NProjOut, uint32_t TPROJMask> bool addProj(const TrackletProjection<TProjType> &proj, const BXType bx, TrackletProjectionMemory<TProjType> projout[NProjOut], int nproj[NProjOut], const bool success);

  template<TC::seed Seed, regionType InnerRegion, regionType OuterRegion, uint32_t TPROJMaskBarrel, uint32_t TPROJMaskDisk> void
    processStubPair(
		    const BXType bx,
		    const ap_uint<7> innerIndex,
		    const AllStub<InnerRegion> &innerStub,
		    const ap_uint<7>  outerIndex,
		    const AllStub<OuterRegion> &outerStub,
		    const TrackletProjection<BARRELPS>::TProjTCID TCID,
		    TrackletProjection<BARRELPS>::TProjTrackletIndex &trackletIndex,
		    TrackletParameterMemory * const trackletParameters,
		    TrackletProjectionMemory<BARRELPS> projout_barrel_ps[N_PROJOUT_BARRELPS],
		    TrackletProjectionMemory<BARREL2S> projout_barrel_2s[N_PROJOUT_BARREL2S],
		    TrackletProjectionMemory<DISK> projout_disk[N_PROJOUT_DISK],
		    int &npar,
		    int nproj_barrel_ps[N_PROJOUT_BARRELPS],
		    int nproj_barrel_2s[N_PROJOUT_BARREL2S],
		    int nproj_disk[N_PROJOUT_DISK]
		    );


}

template<TC::seed Seed> constexpr regionType InnerRegion() {
  return (
    (Seed == TC::L1L2 || Seed == TC::L2L3 || Seed == TC::L3L4 || Seed == TC::L1D1 || Seed == TC::L2D1) ? BARRELPS : (
      (Seed == TC::L5L6) ? BARREL2S : DISK
    )
  );
}
template<TC::seed Seed> constexpr regionType OuterRegion() {
  return (
    (Seed == TC::L1L2 || Seed == TC::L2L3) ? BARRELPS : (
      (Seed == TC::L3L4 || Seed == TC::L5L6) ? BARREL2S : DISK
    )
  );
}
template<TC::seed Seed, TC::itc iTC> constexpr uint8_t NASMemInner();
template<TC::seed Seed, TC::itc iTC> constexpr uint8_t NASMemOuter();
template<TC::seed Seed, TC::itc iTC> constexpr uint8_t NSPMem();
template<TC::seed Seed, TC::itc iTC> constexpr uint16_t ASInnerMask();
template<TC::seed Seed, TC::itc iTC> constexpr uint16_t ASOuterMask();
template<TC::seed Seed, TC::itc iTC> constexpr uint32_t TPROJMaskBarrel();
template<TC::seed Seed, TC::itc iTC> constexpr uint32_t TPROJMaskDisk();

void TrackletProcessor_L1L2D(const BXType bx,
			     const ap_uint<10> lut[2048],
			     const ap_uint<8> regionlut[2048],
			     const ap_uint<1> stubptinnerlut[256],
			     const ap_uint<1> stubptouterlut[256],
			     const AllStubInnerMemory<BARRELPS> innerStubs[2],
			     const AllStubMemory<BARRELPS>* outerStubs,
			     const VMStubTEOuterMemoryCM<BARRELPS> outerVMStubs[6],
			     TrackletParameterMemory * trackletParameters,
			     TrackletProjectionMemory<BARRELPS> projout_barrel_ps[TC::N_PROJOUT_BARRELPS],
			     TrackletProjectionMemory<BARREL2S> projout_barrel_2s[TC::N_PROJOUT_BARREL2S],
			     TrackletProjectionMemory<DISK> projout_disk[TC::N_PROJOUT_DISK]
			     );

////////////////////////////////////////////////////////////////////////////////

#include "TrackletCalculator_calculate_LXLY.h"
#include "TrackletProcessor_parameters.h"

// This function calls calculate_LXLY, defined in
// TrackletCalculator_calculate_LXLY.h, and applies cuts to the results.
template<TC::seed Seed, regionType InnerRegion, regionType OuterRegion> bool
TC::barrelSeeding(const AllStub<InnerRegion> &innerStub, const AllStub<OuterRegion> &outerStub, TC::Types::rinv * const rinv, TrackletParameters::PHI0PAR * const phi0, TC::Types::z0 * const z0, TrackletParameters::TPAR * const t, TC::Types::phiL phiL[4], TC::Types::zL zL[4], TC::Types::der_phiL * const der_phiL, TC::Types::der_zL * const der_zL, TC::Types::flag valid_proj[4], TC::Types::phiD phiD[4], TC::Types::rD rD[4], TC::Types::der_phiD * const der_phiD, TC::Types::der_rD * const der_rD, TC::Types::flag valid_proj_disk[4])
{
  TC::Types::rmean r1mean, r2mean, rproj[4];
  switch (Seed) {
    case TC::L1L2:
      r1mean   = rmean[L1];
      r2mean   = rmean[L2];
      rproj[0] = rmean[L3];
      rproj[1] = rmean[L4];
      rproj[2] = rmean[L5];
      rproj[3] = rmean[L6];
      break;
    case TC::L3L4:
      rproj[0] = rmean[L1];
      rproj[1] = rmean[L2];
      r1mean   = rmean[L3];
      r2mean   = rmean[L4];
      rproj[2] = rmean[L5];
      rproj[3] = rmean[L6];
      break;
    case TC::L5L6:
      rproj[0] = rmean[L1];
      rproj[1] = rmean[L2];
      rproj[2] = rmean[L3];
      rproj[3] = rmean[L4];
      r1mean   = rmean[L5];
      r2mean   = rmean[L6];
      break;
  }
  TC::Types::zmean zproj[4] = {zmean[D1], zmean[D2], zmean[D3], zmean[D4]};
  calculate_LXLY<Seed, InnerRegion, OuterRegion>(
      innerStub.getR(),
      innerStub.getPhi(),
      innerStub.getZ(),
      outerStub.getR(),
      outerStub.getPhi(),
      outerStub.getZ(),
      r1mean,
      r2mean,
      rproj[0],
      rproj[1],
      rproj[2],
      rproj[3],
      zproj[0],
      zproj[1],
      zproj[2],
      zproj[3],

      rinv,
      phi0,
      t,
      z0,
      &phiL[0],
      &phiL[1],
      &phiL[2],
      &phiL[3],
      &zL[0],
      &zL[1],
      &zL[2],
      &zL[3],
      der_phiL,
      der_zL,
      &phiD[0],
      &phiD[1],
      &phiD[2],
      &phiD[3],
      &rD[0],
      &rD[1],
      &rD[2],
      &rD[3],
      der_phiD,
      der_rD
  );

// Determine which layer projections are valid.
  valid_proj: for (ap_uint<3> i = 0; i < 4; i++) {
    valid_proj[i] = true;
    if (zL[i] < -(1 << (TrackletProjection<BARRELPS>::kTProjRZSize - 1)))
      valid_proj[i] = false;
    if (zL[i] >= (1 << (TrackletProjection<BARRELPS>::kTProjRZSize - 1)))
      valid_proj[i] = false;
    if (phiL[i] >= ((1 << TrackletProjection<BARREL2S>::kTProjPhiSize) - 1))
      valid_proj[i] = false;
    if (phiL[i] <= 0)
      valid_proj[i] = false;
    if (rproj[i] < 2048) {
      phiL[i] >>= (TrackletProjection<BARREL2S>::kTProjPhiSize - TrackletProjection<BARRELPS>::kTProjPhiSize);
      if (phiL[i] >= (1 << TrackletProjection<BARRELPS>::kTProjPhiSize) - 1)
        phiL[i] = (1 << TrackletProjection<BARRELPS>::kTProjPhiSize) - 2;
    }
    else
      zL[i] >>= (TrackletProjection<BARRELPS>::kTProjRZSize - TrackletProjection<BARREL2S>::kTProjRZSize);
  }

// Determine which disk projections are valid.
  valid_proj_disk: for (ap_uint<3> i = 0; i < 4; i++) {
    valid_proj_disk[i] = true;
    if (abs(*t) < 512)
      valid_proj_disk[i] = false;
    if (phiD[i] <= 0)
      valid_proj_disk[i] = false;
    if (phiD[i] >= (1 << TrackletProjection<BARRELPS>::kTProjPhiSize) - 1)
      valid_proj_disk[i] = false;
    if (rD[i] < 342 || rD[i] > 2048)
      valid_proj_disk[i] = false;
  }

// Reject tracklets with too high a curvature or with too large a longitudinal
// impact parameter.
  bool success = true;
  if (abs(*rinv) >= rinvcut)
    success = false;
  if (abs(*z0) >= ((Seed == TC::L1L2) ? z0cut_L1L2 : z0cut))
    success = false;

  const ap_int<TrackletParameters::kTParPhi0Size + 2> phicrit = *phi0 - (*rinv<<1);
  const bool keep = (phicrit > 9253) && (phicrit < 56269);
  success = success && keep;

  return success;
}

// Returns a unique identifier assigned to each TC.
template<TC::seed Seed, TC::itc iTC> const TrackletProjection<BARRELPS>::TProjTCID
TC::ID()
{
  return ((TrackletProjection<BARRELPS>::TProjTCID(Seed) << 4) + iTC);
}

// Writes a tracklet projection to the appropriate tracklet projection memory.
template<regionType TProjType, uint8_t NProjOut, uint32_t TPROJMask> bool
TC::addProj(const TrackletProjection<TProjType> &proj, const BXType bx, TrackletProjectionMemory<TProjType> projout[NProjOut], int nproj[NProjOut], const bool success)
{
  bool proj_success = true;

// Reject projections with extreme r/z values.
  if (TProjType != DISK) {
    if ((proj.getRZ() == (-(1 << (TrackletProjection<TProjType>::kTProjRZSize - 1))) || (proj.getRZ() == ((1 << (TrackletProjection<TProjType>::kTProjRZSize - 1)) - 1))))
      proj_success = false;
    if (abs(proj.getRZ()) > 2048)
      proj_success = false;
  }
  else {
    if (proj.getRZ() < 205 || proj.getRZ() > 1911)
      proj_success = false;
  }

// Fill correct TrackletProjectionMemory according to phi bin of projection.
  TC::Types::phiL phi = proj.getPhi() >> (TrackletProjection<TProjType>::kTProjPhiSize - 5);
  if (TProjType == BARRELPS && NProjOut == nproj_L1)
    phi >>= 2;
  else
    phi >>= 3;

  if (NProjOut > 0 && TPROJMask & (0x1 << 0) && success && proj_success && phi == 0)
    projout[0].write_mem(bx, proj, nproj[0]++);
  if (NProjOut > 1 && TPROJMask & (0x1 << 1) && success && proj_success && phi == 1)
    projout[1].write_mem(bx, proj, nproj[1]++);
  if (NProjOut > 2 && TPROJMask & (0x1 << 2) && success && proj_success && phi == 2)
    projout[2].write_mem(bx, proj, nproj[2]++);
  if (NProjOut > 3 && TPROJMask & (0x1 << 3) && success && proj_success && phi == 3)
    projout[3].write_mem(bx, proj, nproj[3]++);
  if (NProjOut > 4 && TPROJMask & (0x1 << 4) && success && proj_success && phi == 4)
    projout[4].write_mem(bx, proj, nproj[4]++);
  if (NProjOut > 5 && TPROJMask & (0x1 << 5) && success && proj_success && phi == 5)
    projout[5].write_mem(bx, proj, nproj[5]++);
  if (NProjOut > 6 && TPROJMask & (0x1 << 6) && success && proj_success && phi == 6)
    projout[6].write_mem(bx, proj, nproj[6]++);
  if (NProjOut > 7 && TPROJMask & (0x1 << 7) && success && proj_success && phi == 7)
    projout[7].write_mem(bx, proj, nproj[7]++);

  return (success && proj_success);
}


// Processes a given stub pair and writes the calculated tracklet parameters
// and tracklet projections to the appropriate memories.
template<TC::seed Seed, regionType InnerRegion, regionType OuterRegion, uint32_t TPROJMaskBarrel, uint32_t TPROJMaskDisk> void
TC::processStubPair(
    const BXType bx,
    const ap_uint<7> innerIndex,
    const AllStub<InnerRegion> &innerStub,
    const ap_uint<7>  outerIndex,
    const AllStub<OuterRegion> &outerStub,
    const TrackletProjection<BARRELPS>::TProjTCID TCID,
    TrackletProjection<BARRELPS>::TProjTrackletIndex &trackletIndex,
    TrackletParameterMemory * const trackletParameters,
    TrackletProjectionMemory<BARRELPS> projout_barrel_ps[N_PROJOUT_BARRELPS],
    TrackletProjectionMemory<BARREL2S> projout_barrel_2s[N_PROJOUT_BARREL2S],
    TrackletProjectionMemory<DISK> projout_disk[N_PROJOUT_DISK],
    int &npar,
    int nproj_barrel_ps[N_PROJOUT_BARRELPS],
    int nproj_barrel_2s[N_PROJOUT_BARREL2S],
    int nproj_disk[N_PROJOUT_DISK]
)
{
  TC::Types::rinv rinv;
  TrackletParameters::PHI0PAR phi0;
  TC::Types::z0 z0;
  TrackletParameters::TPAR t;
  TC::Types::phiL phiL[4];
  TC::Types::zL zL[4];
  TC::Types::der_phiL der_phiL;
  TC::Types::der_zL der_zL;
  TC::Types::flag valid_proj[4];
  TC::Types::phiD phiD[4];
  TC::Types::rD rD[4];
  TC::Types::der_phiD der_phiD;
  TC::Types::der_rD der_rD;
  TC::Types::flag valid_proj_disk[4];
  bool success;

  //std::cout << "barrelSeeding: innerStub phi z r : "<<innerStub.getPhi()<<" "<<innerStub.getZ()<<" "<<innerStub.getR()<<std::endl;
  //std::cout << "barrelSeeding: outerStub phi z r : "<<outerStub.getPhi()<<" "<<outerStub.getZ()<<" "<<outerStub.getR()<<std::endl;


// Calculate the tracklet parameters and projections.
  success = TC::barrelSeeding<Seed, InnerRegion, OuterRegion>(innerStub, outerStub, &rinv, &phi0, &z0, &t, phiL, zL, &der_phiL, &der_zL, valid_proj, phiD, rD, &der_phiD, &der_rD, valid_proj_disk);

  //std::cout << "barrelSeeding: success : "<<success<<std::endl;

// Write the tracklet parameters and projections to the output memories.
  const TrackletParameters tpar(innerIndex, outerIndex, rinv, phi0, z0, t);
  if (success) trackletParameters->write_mem(bx, tpar, npar++);

  bool addL3 = false, addL4 = false, addL5 = false, addL6 = false;

  switch (Seed) {
    case TC::L1L2:
      {
        const TrackletProjection<BARRELPS> tproj_L3(TCID, trackletIndex, phiL[0], zL[0], der_phiL, der_zL);
        const TrackletProjection<BARREL2S> tproj_L4(TCID, trackletIndex, phiL[1], zL[1], der_phiL, der_zL);
        const TrackletProjection<BARREL2S> tproj_L5(TCID, trackletIndex, phiL[2], zL[2], der_phiL, der_zL);
        const TrackletProjection<BARREL2S> tproj_L6(TCID, trackletIndex, phiL[3], zL[3], der_phiL, der_zL);

        addL3 = TC::addProj<BARRELPS, nproj_L3, ((TPROJMaskBarrel & mask_L3) >> shift_L3)> (tproj_L3, bx, &projout_barrel_ps[L3PHIA], &nproj_barrel_ps[L3PHIA], success && valid_proj[0]);
        addL4 = TC::addProj<BARREL2S, nproj_L4, ((TPROJMaskBarrel & mask_L4) >> shift_L4)> (tproj_L4, bx, &projout_barrel_2s[L4PHIA], &nproj_barrel_2s[L4PHIA], success && valid_proj[1]);
        addL5 = TC::addProj<BARREL2S, nproj_L5, ((TPROJMaskBarrel & mask_L5) >> shift_L5)> (tproj_L5, bx, &projout_barrel_2s[L5PHIA], &nproj_barrel_2s[L5PHIA], success && valid_proj[2]);
        addL6 = TC::addProj<BARREL2S, nproj_L6, ((TPROJMaskBarrel & mask_L6) >> shift_L6)> (tproj_L6, bx, &projout_barrel_2s[L6PHIA], &nproj_barrel_2s[L6PHIA], success && valid_proj[3]);
      }

      break;

    case TC::L3L4:
      {
        const TrackletProjection<BARRELPS> tproj_L1(TCID, trackletIndex, phiL[0], zL[0], der_phiL, der_zL);
        const TrackletProjection<BARRELPS> tproj_L2(TCID, trackletIndex, phiL[1], zL[1], der_phiL, der_zL);
        const TrackletProjection<BARREL2S> tproj_L5(TCID, trackletIndex, phiL[2], zL[2], der_phiL, der_zL);
        const TrackletProjection<BARREL2S> tproj_L6(TCID, trackletIndex, phiL[3], zL[3], der_phiL, der_zL);

        TC::addProj<BARRELPS, nproj_L1, ((TPROJMaskBarrel & mask_L1) >> shift_L1)> (tproj_L1, bx, &projout_barrel_ps[L1PHIA], &nproj_barrel_ps[L1PHIA], success && valid_proj[0]);
        TC::addProj<BARRELPS, nproj_L2, ((TPROJMaskBarrel & mask_L2) >> shift_L2)> (tproj_L2, bx, &projout_barrel_ps[L2PHIA], &nproj_barrel_ps[L2PHIA], success && valid_proj[1]);
        addL3 = addL4 = true;
        addL5 = TC::addProj<BARREL2S, nproj_L5, ((TPROJMaskBarrel & mask_L5) >> shift_L5)> (tproj_L5, bx, &projout_barrel_2s[L5PHIA], &nproj_barrel_2s[L5PHIA], success && valid_proj[2]);
        addL6 = TC::addProj<BARREL2S, nproj_L6, ((TPROJMaskBarrel & mask_L6) >> shift_L6)> (tproj_L6, bx, &projout_barrel_2s[L6PHIA], &nproj_barrel_2s[L6PHIA], success && valid_proj[3]);
      }

      break;

    case TC::L5L6:
      {
        const TrackletProjection<BARRELPS> tproj_L1(TCID, trackletIndex, phiL[0], zL[0], der_phiL, der_zL);
        const TrackletProjection<BARRELPS> tproj_L2(TCID, trackletIndex, phiL[1], zL[1], der_phiL, der_zL);
        const TrackletProjection<BARRELPS> tproj_L3(TCID, trackletIndex, phiL[2], zL[2], der_phiL, der_zL);
        const TrackletProjection<BARREL2S> tproj_L4(TCID, trackletIndex, phiL[3], zL[3], der_phiL, der_zL);

        TC::addProj<BARRELPS, nproj_L1, ((TPROJMaskBarrel & mask_L1) >> shift_L1)> (tproj_L1, bx, &projout_barrel_ps[L1PHIA], &nproj_barrel_ps[L1PHIA], success && valid_proj[0]);
        TC::addProj<BARRELPS, nproj_L2, ((TPROJMaskBarrel & mask_L2) >> shift_L2)> (tproj_L2, bx, &projout_barrel_ps[L2PHIA], &nproj_barrel_ps[L2PHIA], success && valid_proj[1]);
        addL3 = TC::addProj<BARRELPS, nproj_L3, ((TPROJMaskBarrel & mask_L3) >> shift_L3)> (tproj_L3, bx, &projout_barrel_ps[L3PHIA], &nproj_barrel_ps[L3PHIA], success && valid_proj[2]);
        addL4 = TC::addProj<BARREL2S, nproj_L4, ((TPROJMaskBarrel & mask_L4) >> shift_L4)> (tproj_L4, bx, &projout_barrel_2s[L4PHIA], &nproj_barrel_2s[L4PHIA], success && valid_proj[3]);
        addL5 = addL6 = true;
      }

      break;
  }

  const TrackletProjection<DISK> tproj_D1(TCID, trackletIndex, phiD[0], rD[0], der_phiD, der_rD);
  const TrackletProjection<DISK> tproj_D2(TCID, trackletIndex, phiD[1], rD[1], der_phiD, der_rD);
  const TrackletProjection<DISK> tproj_D3(TCID, trackletIndex, phiD[2], rD[2], der_phiD, der_rD);
  const TrackletProjection<DISK> tproj_D4(TCID, trackletIndex, phiD[3], rD[3], der_phiD, der_rD);

  TC::addProj<DISK, nproj_D1, ((TPROJMaskDisk & mask_D1) >> shift_D1)> (tproj_D1, bx, &projout_disk[D1PHIA], &nproj_disk[D1PHIA], success && valid_proj_disk[0] && !addL6);
  TC::addProj<DISK, nproj_D2, ((TPROJMaskDisk & mask_D2) >> shift_D2)> (tproj_D2, bx, &projout_disk[D2PHIA], &nproj_disk[D2PHIA], success && valid_proj_disk[1] && !addL5);
  TC::addProj<DISK, nproj_D3, ((TPROJMaskDisk & mask_D3) >> shift_D3)> (tproj_D3, bx, &projout_disk[D3PHIA], &nproj_disk[D3PHIA], success && valid_proj_disk[2] && !addL4);
  TC::addProj<DISK, nproj_D4, ((TPROJMaskDisk & mask_D4) >> shift_D4)> (tproj_D4, bx, &projout_disk[D4PHIA], &nproj_disk[D4PHIA], success && valid_proj_disk[3] && !addL3);

  if (success) trackletIndex++;
}

// This is the primary interface for the TrackletProcessor.
template<
TC::seed Seed, // seed layer combination (TC::L1L2, TC::L3L4, etc.)
  TC::itc iTC, // letter at the end of the TC name (TC_L1L2A and TC_L5L6A have
// the same iTC); generally indicates the region of the phi sector
             // being processed
  uint8_t NTEBuffer, //number of TE buffers
  uint8_t NTEUnits, //number of TE units
  regionType InnerRegion, // region type of the inner stubs
  regionType OuterRegion, // region type of the outer stubs
  uint8_t NASMemInner, // number of inner all-stub memories
  uint16_t N // maximum number of stub pairs processed
> void
TrackletProcessor(
    const BXType bx,
    const ap_uint<10> lut[2048],
    const ap_uint<8> regionlut[2048],
    const ap_uint<1> stubptinnerlut[256],
    const ap_uint<1> stubptouterlut[256],
    const AllStubInnerMemory<InnerRegion> innerStubs[NASMemInner],
    const AllStubMemory<OuterRegion>* outerStubs,
    const VMStubTEOuterMemoryCM<OuterRegion> outerVMStubs[6],
    TrackletParameterMemory * const trackletParameters,
    TrackletProjectionMemory<BARRELPS> projout_barrel_ps[TC::N_PROJOUT_BARRELPS],
    TrackletProjectionMemory<BARREL2S> projout_barrel_2s[TC::N_PROJOUT_BARREL2S],
    TrackletProjectionMemory<DISK> projout_disk[TC::N_PROJOUT_DISK]
)
{

  static_assert(Seed == TC::L1L2, "Only L1L2 seeds have been implemented so far.");

  int npar = 0;
  int nproj_barrel_ps[TC::N_PROJOUT_BARRELPS] = {0};
  int nproj_barrel_2s[TC::N_PROJOUT_BARREL2S] = {0};
  int nproj_disk[TC::N_PROJOUT_DISK] = {0};
#pragma HLS array_partition variable=nproj_barrel_ps complete
#pragma HLS array_partition variable=nproj_barrel_2s complete
#pragma HLS array_partition variable=nproj_disk complete

// Clear all output memories before starting.
  trackletParameters->clear(bx);
 clear_barrel_ps: for (unsigned i = 0; i < TC::N_PROJOUT_BARRELPS; i++)
#pragma HLS unroll
    if (TPROJMaskBarrel<Seed, iTC>() & (0x1 << i))
      projout_barrel_ps[i].clear();
 clear_barrel_2s: for (unsigned i = 0; i < TC::N_PROJOUT_BARREL2S; i++)
#pragma HLS unroll
    if (TPROJMaskBarrel<Seed, iTC>() & (0x1 << (i + TC::N_PROJOUT_BARRELPS)))
      projout_barrel_2s[i].clear();
 clear_disk: for (unsigned i = 0; i < TC::N_PROJOUT_DISK; i++)
#pragma HLS unroll
    if (TPROJMaskDisk<Seed, iTC>() & (0x1 << i))
      projout_disk[i].clear();


  TEBuffer tebuffer[NTEBuffer];
#pragma HLS array_partition variable=tebuffer complete
  //Need to generalize this
  static_assert(NASMemInner == 2, "Only handling two inner AS memories");
  tebuffer[0].setMemBegin(0);
  tebuffer[0].setMemEnd(1);
  tebuffer[1].setMemBegin(1);
  tebuffer[1].setMemEnd(2);
  
 reset_tebuffers: for (unsigned i = 0; i < NTEBuffer; i++)
#pragma HLS unroll
    tebuffer[i].reset();

  TrackletEngineUnit<BARRELPS> teunits[NTEUnits];
#pragma HLS array_partition variable=teunits complete dim=0

 reset_teunits: for (unsigned i = 0; i < NTEUnits; i++) {
#pragma HLS unroll
    teunits[i].reset();
  }

  TrackletProjection<BARRELPS>::TProjTrackletIndex trackletIndex = 0;

 istep_loop: for(unsigned istep=0;istep<108;istep++) {
#pragma HLS pipeline II=1
#pragma HLS dependence variable=tebuffer[0].readptr_ intra false 
#pragma HLS dependence variable=tebuffer[1].readptr_ intra false 
#pragma HLS dependence variable=tebuffer[0].readptr_ inter false 
#pragma HLS dependence variable=tebuffer[1].readptr_ inter false 
#pragma HLS dependence variable=tebuffer[0].writeptr_ intra false 
#pragma HLS dependence variable=tebuffer[1].writeptr_ intra false 
#pragma HLS dependence variable=tebuffer[0].writeptr_ inter false 
#pragma HLS dependence variable=tebuffer[1].writeptr_ inter false 
#pragma HLS dependence variable=tebuffer[0].buffer_ inter false 
#pragma HLS dependence variable=tebuffer[1].buffer_ inter false 
#pragma HLS dependence variable=tebuffer[0].buffer_ intra false 
#pragma HLS dependence variable=tebuffer[1].buffer_ intra false 

#pragma HLS dependence variable=teunits[0].slot_ inter false 
#pragma HLS dependence variable=teunits[0].slot_ intra false 
#pragma HLS dependence variable=teunits[0].ireg_ inter false 
#pragma HLS dependence variable=teunits[0].ireg_ intra false 
#pragma HLS dependence variable=teunits[0].istub_ inter false 
#pragma HLS dependence variable=teunits[0].istub_ intra false 
#pragma HLS dependence variable=teunits[0].next_ inter false 
#pragma HLS dependence variable=teunits[0].next_ intra false 
#pragma HLS dependence variable=teunits[0].bx_ inter false 
#pragma HLS dependence variable=teunits[0].bx_ intra false 
#pragma HLS dependence variable=teunits[0].nstubs_ inter false 
#pragma HLS dependence variable=teunits[0].nstubs_ intra false 
#pragma HLS dependence variable=teunits[0].idle_ inter false 
#pragma HLS dependence variable=teunits[0].idle_ intra false 
#pragma HLS dependence variable=teunits[0].writeindex_ inter false 
#pragma HLS dependence variable=teunits[0].writeindex_ intra false 
#pragma HLS dependence variable=teunits[0].readindex_ inter false 
#pragma HLS dependence variable=teunits[0].readindex_ intra false 
#pragma HLS dependence variable=teunits[0].innerstub_ inter false 
#pragma HLS dependence variable=teunits[0].innerstub_ intra false 
#pragma HLS dependence variable=teunits[0].memstubs_ inter false 
#pragma HLS dependence variable=teunits[0].memstubs_ intra false 
#pragma HLS dependence variable=teunits[0].rzbindiffmax_ inter false 
#pragma HLS dependence variable=teunits[0].rzbindiffmax_ intra false 
#pragma HLS dependence variable=teunits[0].rzbinfirst_ inter false 
#pragma HLS dependence variable=teunits[0].rzbinfirst_ intra false 

#pragma HLS dependence variable=teunits[1].slot_ inter false 
#pragma HLS dependence variable=teunits[1].slot_ intra false 
#pragma HLS dependence variable=teunits[1].ireg_ inter false 
#pragma HLS dependence variable=teunits[1].ireg_ intra false 
#pragma HLS dependence variable=teunits[1].istub_ inter false 
#pragma HLS dependence variable=teunits[1].istub_ intra false 
#pragma HLS dependence variable=teunits[1].next_ inter false 
#pragma HLS dependence variable=teunits[1].next_ intra false 
#pragma HLS dependence variable=teunits[1].bx_ inter false 
#pragma HLS dependence variable=teunits[1].bx_ intra false 
#pragma HLS dependence variable=teunits[1].nstubs_ inter false 
#pragma HLS dependence variable=teunits[1].nstubs_ intra false 
#pragma HLS dependence variable=teunits[1].idle_ inter false 
#pragma HLS dependence variable=teunits[1].idle_ intra false 
#pragma HLS dependence variable=teunits[1].writeindex_ inter false 
#pragma HLS dependence variable=teunits[1].writeindex_ intra false 
#pragma HLS dependence variable=teunits[1].readindex_ inter false 
#pragma HLS dependence variable=teunits[1].readindex_ intra false 
#pragma HLS dependence variable=teunits[1].innerstub_ inter false 
#pragma HLS dependence variable=teunits[1].innerstub_ intra false 
#pragma HLS dependence variable=teunits[1].memstubs_ inter false 
#pragma HLS dependence variable=teunits[1].memstubs_ intra false 
#pragma HLS dependence variable=teunits[1].rzbindiffmax_ inter false 
#pragma HLS dependence variable=teunits[1].rzbindiffmax_ intra false 
#pragma HLS dependence variable=teunits[1].rzbinfirst_ inter false 
#pragma HLS dependence variable=teunits[1].rzbinfirst_ intra false 

#pragma HLS dependence variable=teunits[2].slot_ inter false 
#pragma HLS dependence variable=teunits[2].slot_ intra false 
#pragma HLS dependence variable=teunits[2].ireg_ inter false 
#pragma HLS dependence variable=teunits[2].ireg_ intra false 
#pragma HLS dependence variable=teunits[2].istub_ inter false 
#pragma HLS dependence variable=teunits[2].istub_ intra false 
#pragma HLS dependence variable=teunits[2].next_ inter false 
#pragma HLS dependence variable=teunits[2].next_ intra false 
#pragma HLS dependence variable=teunits[2].bx_ inter false 
#pragma HLS dependence variable=teunits[2].bx_ intra false 
#pragma HLS dependence variable=teunits[2].nstubs_ inter false 
#pragma HLS dependence variable=teunits[2].nstubs_ intra false 
#pragma HLS dependence variable=teunits[2].idle_ inter false 
#pragma HLS dependence variable=teunits[2].idle_ intra false 
#pragma HLS dependence variable=teunits[2].writeindex_ inter false 
#pragma HLS dependence variable=teunits[2].writeindex_ intra false 
#pragma HLS dependence variable=teunits[2].readindex_ inter false 
#pragma HLS dependence variable=teunits[2].readindex_ intra false 
#pragma HLS dependence variable=teunits[2].innerstub_ inter false 
#pragma HLS dependence variable=teunits[2].innerstub_ intra false 
#pragma HLS dependence variable=teunits[2].memstubs_ inter false 
#pragma HLS dependence variable=teunits[2].memstubs_ intra false 
#pragma HLS dependence variable=teunits[2].rzbindiffmax_ inter false 
#pragma HLS dependence variable=teunits[2].rzbindiffmax_ intra false 
#pragma HLS dependence variable=teunits[2].rzbinfirst_ inter false 
#pragma HLS dependence variable=teunits[2].rzbinfirst_ intra false 

#pragma HLS dependence variable=teunits[3].slot_ inter false 
#pragma HLS dependence variable=teunits[3].slot_ intra false 
#pragma HLS dependence variable=teunits[3].ireg_ inter false 
#pragma HLS dependence variable=teunits[3].ireg_ intra false 
#pragma HLS dependence variable=teunits[3].istub_ inter false 
#pragma HLS dependence variable=teunits[3].istub_ intra false 
#pragma HLS dependence variable=teunits[3].next_ inter false 
#pragma HLS dependence variable=teunits[3].next_ intra false 
#pragma HLS dependence variable=teunits[3].bx_ inter false 
#pragma HLS dependence variable=teunits[3].bx_ intra false 
#pragma HLS dependence variable=teunits[3].nstubs_ inter false 
#pragma HLS dependence variable=teunits[3].nstubs_ intra false 
#pragma HLS dependence variable=teunits[3].idle_ inter false 
#pragma HLS dependence variable=teunits[3].idle_ intra false 
#pragma HLS dependence variable=teunits[3].writeindex_ inter false 
#pragma HLS dependence variable=teunits[3].writeindex_ intra false 
#pragma HLS dependence variable=teunits[3].readindex_ inter false 
#pragma HLS dependence variable=teunits[3].readindex_ intra false 
#pragma HLS dependence variable=teunits[3].innerstub_ inter false 
#pragma HLS dependence variable=teunits[3].innerstub_ intra false 
#pragma HLS dependence variable=teunits[3].memstubs_ inter false 
#pragma HLS dependence variable=teunits[3].memstubs_ intra false 
#pragma HLS dependence variable=teunits[3].rzbindiffmax_ inter false 
#pragma HLS dependence variable=teunits[3].rzbindiffmax_ intra false 
#pragma HLS dependence variable=teunits[3].rzbinfirst_ inter false 
#pragma HLS dependence variable=teunits[3].rzbinfirst_ intra false 

#pragma HLS dependence variable=teunits[4].slot_ inter false 
#pragma HLS dependence variable=teunits[4].slot_ intra false 
#pragma HLS dependence variable=teunits[4].ireg_ inter false 
#pragma HLS dependence variable=teunits[4].ireg_ intra false 
#pragma HLS dependence variable=teunits[4].istub_ inter false 
#pragma HLS dependence variable=teunits[4].istub_ intra false 
#pragma HLS dependence variable=teunits[4].next_ inter false 
#pragma HLS dependence variable=teunits[4].next_ intra false 
#pragma HLS dependence variable=teunits[4].bx_ inter false 
#pragma HLS dependence variable=teunits[4].bx_ intra false 
#pragma HLS dependence variable=teunits[4].nstubs_ inter false 
#pragma HLS dependence variable=teunits[4].nstubs_ intra false 
#pragma HLS dependence variable=teunits[4].idle_ inter false 
#pragma HLS dependence variable=teunits[4].idle_ intra false 
#pragma HLS dependence variable=teunits[4].writeindex_ inter false 
#pragma HLS dependence variable=teunits[4].writeindex_ intra false 
#pragma HLS dependence variable=teunits[4].readindex_ inter false 
#pragma HLS dependence variable=teunits[4].readindex_ intra false 
#pragma HLS dependence variable=teunits[4].innerstub_ inter false 
#pragma HLS dependence variable=teunits[4].innerstub_ intra false 
#pragma HLS dependence variable=teunits[4].memstubs_ inter false 
#pragma HLS dependence variable=teunits[4].memstubs_ intra false 
#pragma HLS dependence variable=teunits[4].rzbindiffmax_ inter false 
#pragma HLS dependence variable=teunits[4].rzbindiffmax_ intra false 
#pragma HLS dependence variable=teunits[4].rzbinfirst_ inter false 
#pragma HLS dependence variable=teunits[4].rzbinfirst_ intra false 

#pragma HLS dependence variable=teunits[5].slot_ inter false 
#pragma HLS dependence variable=teunits[5].slot_ intra false 
#pragma HLS dependence variable=teunits[5].ireg_ inter false 
#pragma HLS dependence variable=teunits[5].ireg_ intra false 
#pragma HLS dependence variable=teunits[5].istub_ inter false 
#pragma HLS dependence variable=teunits[5].istub_ intra false 
#pragma HLS dependence variable=teunits[5].next_ inter false 
#pragma HLS dependence variable=teunits[5].next_ intra false 
#pragma HLS dependence variable=teunits[5].bx_ inter false 
#pragma HLS dependence variable=teunits[5].bx_ intra false 
#pragma HLS dependence variable=teunits[5].nstubs_ inter false 
#pragma HLS dependence variable=teunits[5].nstubs_ intra false 
#pragma HLS dependence variable=teunits[5].idle_ inter false 
#pragma HLS dependence variable=teunits[5].idle_ intra false 
#pragma HLS dependence variable=teunits[5].writeindex_ inter false 
#pragma HLS dependence variable=teunits[5].writeindex_ intra false 
#pragma HLS dependence variable=teunits[5].readindex_ inter false 
#pragma HLS dependence variable=teunits[5].readindex_ intra false 
#pragma HLS dependence variable=teunits[5].innerstub_ inter false 
#pragma HLS dependence variable=teunits[5].innerstub_ intra false 
#pragma HLS dependence variable=teunits[5].memstubs_ inter false 
#pragma HLS dependence variable=teunits[5].memstubs_ intra false 
#pragma HLS dependence variable=teunits[5].rzbindiffmax_ inter false 
#pragma HLS dependence variable=teunits[5].rzbindiffmax_ intra false 
#pragma HLS dependence variable=teunits[5].rzbinfirst_ inter false 
#pragma HLS dependence variable=teunits[5].rzbinfirst_ intra false 


    //std::cout << "************************ TP istep = " << istep << " *********************"<< std::endl;
    //status_teunits: for (unsigned int k = 0 ; k < NTEUnits; k++){
    // std::cout << "TE["<<k<<"] i,e: "<<teunits[k].idle()<<" "<<teunits[k].empty()<<"  ";
    //}
    //std::cout <<std::endl;

    //
    // In this first step we check if there are stubs to be fit
    //

    int iTE=-1;
  process_teunits: for (unsigned int k = 0 ; k < NTEUnits; k++){
#pragma HLS unroll
      if (!teunits[k].empty()){
	iTE=k;
      }
    }
    
    if (iTE!=-1) {
      
      ap_uint<36> innerStub;
      ap_uint<7> innerIndex;
      ap_uint<8> finephi;
      ap_uint<7> outerIndex;
      (outerIndex, innerStub, innerIndex, finephi)=teunits[iTE].read();

      const TrackletProjection<BARRELPS>::TProjTCID TCID(3);
      
      const auto &outerStub = outerStubs->read_mem(bx, outerIndex);
      
      TC::processStubPair<Seed, InnerRegion, OuterRegion, TPROJMaskBarrel<Seed, iTC>(), TPROJMaskDisk<Seed, iTC>()>(bx, innerIndex, AllStub<BARRELPS>(innerStub), outerIndex, outerStub, TCID, trackletIndex, trackletParameters, projout_barrel_ps, projout_barrel_2s, projout_disk, npar, nproj_barrel_ps, nproj_barrel_2s, nproj_disk);
      
    }
     

    //
    // Second step
    // 


    int iTEUnit=-1;
    int iTEBuff=-1;
  step_teunits: for (unsigned int k = 0 ; k < NTEUnits; k++){
#pragma HLS unroll
      if (!teunits[k].idle()) {   
	teunits[k].step(outerVMStubs[k],stubptinnerlut,stubptouterlut);
      } else {      
      check_tebuffers: for (unsigned i = 0; i < NTEBuffer; i++){
#pragma HLS unroll
	  if (tebuffer[i].empty()) {
	    continue;
	  }
	  if (iTEUnit==-1) {
	    //std::cout << "istep = "<<istep<<" tebuffer " << i << " is not empty and initializing teunit " << k << std::endl; 
	    iTEUnit=k;
	    iTEBuff=i;
	  }
	}
      }
    }
    
    if (iTEUnit!=-1) {
      TEData tedatatmp(tebuffer[iTEBuff].read());
      teunits[iTEUnit].init(bx,
			    tedatatmp.getAllStub(),
			    tedatatmp.getNStub(),
			    tedatatmp.getStart(),
			    tedatatmp.getrzbinfirst(),
			    tedatatmp.getrzdiffmax());
    }

    //
    // Third step
    //

  process_tebuffers: for (unsigned i = 0; i < NTEBuffer; i++){
#pragma HLS unroll
      auto& imem=tebuffer[i].getMem();
      auto imemend=tebuffer[i].getMemEnd();
      if (imem>=imemend)  //could make just ==
	continue;
      if (tebuffer[i].full())
	continue;
      auto& istub=tebuffer[i].getIStub();
      if (istub>=innerStubs[imem].getEntries(bx))
	continue;
      auto stub=innerStubs[imem].read_mem(bx,istub);

      auto phi=stub.getPhi();
      auto z=stub.getZ();
      auto r=stub.getR();
      auto bend=stub.getBend();

      auto innerfinephi=stub.getFinePhi();
      
      int nbitszfinebintable=7;
      auto indexz=z.range(z.length()-1,z.length()-nbitszfinebintable);

      int nbitsrfinebintable=4;
      auto indexr=r.range(r.length()-1,r.length()-nbitsrfinebintable);

      ap_uint<3> rzfinebinfirst;
      ap_uint<1> usenext;
      ap_uint<3> start;
      ap_uint<3> rzdiffmax;

      (rzdiffmax,start, usenext, rzfinebinfirst) = lut[(indexz,indexr)];
      if (lut[(indexz,indexr)]!=1023) {

	auto useregion=regionlut[(innerfinephi,bend)];

	ap_uint<64> nstubs(0);

	int nmem=0;
      ireg_loop: for(unsigned int ireg=0;ireg<8;ireg++) {
#pragma HLS unroll
	  if (!useregion.test(ireg))
	    continue;
	next_loop: for(unsigned inext=0;inext<2;inext++) {
#pragma HLS unroll
#pragma HLS loop_flatten
	    if (inext>usenext)
	      continue;
	    ap_uint<1> next(inext);
	    unsigned ibin=start+next;
	    ap_uint<4> numstubs=outerVMStubs[i].getEntries(bx,(ap_uint<3>(ireg),ap_uint<3>(ibin)));
	    if (numstubs!=0){
#ifndef __SYNTHESIS__
	      assert(nmem!=8);
#endif
	      nstubs.range(nmem*8+7,nmem*8)=((ap_uint<3>(ireg),next),numstubs);
	      nmem++;
	    }
	  }
	}
	
	if (nmem!=0) { //FIXME - test
	  TEData tedatatmp(nstubs,rzfinebinfirst,start,rzdiffmax,stub.raw());
	  tebuffer[i].store(tedatatmp.raw());
	}
      }

      istub++;
      if (istub>=innerStubs[imem].getEntries(bx)) {
	istub=0;
	imem++;
      }

    }

    
    
  } //end of istep
  
}

#endif