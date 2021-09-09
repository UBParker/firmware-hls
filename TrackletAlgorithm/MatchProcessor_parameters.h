#ifndef TrackletAlgorithm_MatchProcessor_parameters_h
#define TrackletAlgorithm_MatchProcessor_parameters_h

template<int kNBitsBuffer>
static const ap_uint<(1 << (2 * kNBitsBuffer))> nearFullUnit() {
  ap_uint<(1 << (2 * kNBitsBuffer))> lut;
  for(int i = 0; i < (1 << (2 * kNBitsBuffer)); ++i) {
#pragma HLS unroll
    ap_uint<kNBitsBuffer> wptr, rptr;
    ap_uint<2 * kNBitsBuffer> address(i);
    (rptr,wptr) = address;
    auto wptr1 = wptr+1;
    auto wptr2 = wptr+2;
    bool result = wptr1==rptr || wptr2==rptr;
    lut[i] = result;
  }
  return lut;
}

template<int kNBitsBuffer>
static const ap_uint<(1 << (2 * kNBitsBuffer))> nearFull3Unit() {
  ap_uint<(1 << (2 * kNBitsBuffer))> lut;
  for(int i = 0; i < (1 << (2 * kNBitsBuffer)); ++i) {
#pragma HLS unroll
    ap_uint<kNBitsBuffer> wptr, rptr;
    ap_uint<2 * kNBitsBuffer> address(i);
    (rptr,wptr) = address;
    ap_uint<kNBitsBuffer> wptr1 = wptr+1;
    ap_uint<kNBitsBuffer> wptr2 = wptr+2;
    ap_uint<kNBitsBuffer> wptr3 = wptr+3;
    bool result = wptr1==rptr || wptr2==rptr || wptr3==rptr;
    lut[i] = result;
  }
  return lut;
}

template<int kNBitsBuffer>
static const ap_uint<(1 << (2 * kNBitsBuffer))> emptyUnit() {
  ap_uint<(1 << (2 * kNBitsBuffer))> lut;
  for(int i = 0; i < (1 << (2 * kNBitsBuffer)); ++i) {
#pragma HLS unroll
    ap_uint<kNBitsBuffer> wptr, rptr;
    ap_uint<2 * kNBitsBuffer> address(i);
    (rptr,wptr) = address;
    bool result = wptr==rptr;
    lut[i] = result;
  }
  return lut;
}

template<int kNBitsBuffer>
static const ap_uint<(1 << (2 * kNBitsBuffer))> geq() {
  ap_uint<(1 << (2 * kNBitsBuffer))> lut;
  for(int i = 0; i < (1 << (2 * kNBitsBuffer)); ++i) {
#pragma HLS unroll
    ap_uint<kNBitsBuffer> istub, nstubs;
    ap_uint<2 * kNBitsBuffer> address(i);
    (nstubs,istub) = address;
    bool result = istub+1>=nstubs;
    lut[i] = result;
  }
  return lut;
}

template<int kNBitsBuffer>
static const ap_uint<(1 << kNBitsBuffer)> nextUnit() {
  ap_uint<(1 << kNBitsBuffer)> lut;
  for(int i = 0; i < (1 << kNBitsBuffer); ++i) {
#pragma HLS unroll
    ap_uint<kNBitsBuffer> ptr(i);
    lut[i] = ptr+1;
  }
  return lut;
}

template<int nbits, int max, bool lessThan>
static const ap_uint<1 << nbits> isLessThanSize() {
  ap_uint<1 << nbits> tab(0);
  ap_uint<nbits> Max(max);
  ap_uint<nbits> Min(-max);
  for(int i = 0; i < 1<<nbits; ++i) {
#pragma HLS unroll
    if(lessThan) {
      if(i <= Max || i >= Min) tab[i] = 1;
    }
    else {
      if(i < Max || i > Min) tab[i] = 1;
    }
  }
  return tab;
}

template<int nbits, int max, bool lessThan, int proj, int stub>
static const ap_uint<1 << 2*nbits> isLessThanSize() {
  ap_uint<1 << 2*nbits> tab(0);
  ap_uint<nbits> Max(max);
  ap_uint<nbits> Min(-max);
  for(int i = 0; i < 1<<2*nbits; ++i) {
#pragma HLS unroll
    ap_uint<proj> projphi;
    ap_uint<stub> stubphi;
    ap_uint<proj+stub> address(i);
    (projphi,stubphi) = address;
    ap_uint<nbits> result = projphi - stubphi;
    if(lessThan) {
      if(result <= Max || result >= Min) tab[i] = 1;
    }
    else {
      if(result < Max || result > Min) tab[i] = 1;
    }
  }
  return tab;
}

static void readTable(const TF::layerDisk &L, ap_uint<1> table[]){

  if (L==TF::L1) {
    bool tmp[256]=
#include "../emData/ME/tables/METable_L1.tab"
    for (int i=0;i<256;++i){
#pragma HLS unroll
      table[i]=tmp[i];
    }
  }

/* FIXME uncomment these out when testing L2,L5, and L6. Need to be added to download.sh to work.
  if (L==TF::L2) {
    bool tmp[256]=
#include "../emData/ME/tables/METable_L2.tab"
    for (int i=0;i<256;++i){
#pragma HLS unroll
      table[i]=tmp[i];
    }
  }
*/

  if (L==TF::L3) {
    bool tmp[256]=
#include "../emData/MP/tables/METable_L3.tab"
    for (int i=0;i<256;++i){
#pragma HLS unroll
      table[i]=tmp[i];
    }
  }

  if (L==TF::L4) {
    bool tmp[512]=
#include "../emData/ME/tables/METable_L4.tab"
    for (int i=0;i<512;++i){
#pragma HLS unroll
      table[i]=tmp[i];
    }
  }

/*
  if (L==TF::L5) {
    bool tmp[512]=
#include "../emData/ME/tables/METable_L5.tab"
#pragma HLS unroll
    for (int i=0;i<512;++i){
      table[i]=tmp[i];
    }
  }

  if (L==TF::L6) {
    bool tmp[512]=
#include "../emData/ME/tables/METable_L6.tab"
#pragma HLS unroll
    for (int i=0;i<512;++i){
      table[i]=tmp[i];
    }
  }
*/
}

#endif
