#ifndef MATCHENGINEUNIT_H
#define MATCHENGINEUNIT_H

#include "Constants.h"
#include "CandidateMatchMemory.h"
#include "TrackletProjectionMemory.h"
#include "VMStubMEMemoryCM.h"
#include "VMProjectionMemory.h"
#include "ProjectionRouterBuffer.h"
#include "AllStubMemory.h"
#include "AllProjectionMemory.h"
#include "FullMatchMemory.h"
#include "hls_math.h"
#include <iostream>
#include <fstream>
#include <bitset>

template<int VMProjType> class MatchEngineUnitBase {};

template<>
class MatchEngineUnitBase<BARREL> {
 public:
  enum BitWidths{
    kNBitsBuffer=7,
    kNMatchEngines=8
  };
};

template<int VMSMEType, int VMProjType, int VMPTYPE>
class MatchEngineUnit : public MatchEngineUnitBase<VMProjType> {

 public:
  typedef ap_uint<VMStubMECMBase<VMSMEType>::kVMSMEIDSize> STUBID;
  typedef ap_uint<kNBits_MemAddrBinned+2> NSTUBS;
  typedef ap_uint<MatchEngineUnitBase<VMProjType>::kNBitsBuffer> INDEX;

inline MatchEngineUnit() {
#pragma HLS inline
  nstubs=0;
  ptr = 0;
  unit_ = -1;
  idle_ = true;
  ready_ = false;
  done_ = false;
  MEU_INIT: for(int i = 0; i < 1<<MatchEngineUnitBase<VMProjType>::kNBitsBuffer; ++i) {
    #pragma HLS unroll
    stubids[i] = 0;
  }

}

/*
inline MatchEngineUnit(const MatchEngineUnit& meu) {
#pragma HLS inline
  writeindex = meu.writeindex;
  readindex = meu.readindex;
  ptr = meu.ptr;
  nstubs = meu.nstubs;
  idle_ = meu.idle_;
  done_ = meu.done_;
  istep_ = meu.istep_;
  ivmphi = meu.ivmphi;
  unit_ = meu.unit_;
  bx = meu.bx;
  istub = meu.istub;
  MEU_INIT: for(int i = 0; i < 1<<MatchEngineUnitBase<VMProjType>::kNBitsBuffer; ++i) {
    #pragma HLS unroll
    stubids[i] = meu.stubids[i];
  }
  stubids = meu.stubids;
  projfinezadj = meu.projfinezadj;
  tcid = meu.tcid;
  isPSseed = meu.isPSseed;
  zbin = meu.zbin;
  projrinv = meu.projrinv;
  projindex = meu.projindex;
  projbuffer = meu.projbuffer;
}
*/

inline void operator=(const MatchEngineUnit& meu) {
#pragma HLS inline
  writeindex = meu.writeindex;
  readindex = meu.readindex;
  ptr = meu.ptr;
  nstubs = meu.nstubs;
  idle_ = meu.idle_;
  done_ = meu.done_;
  istep_ = meu.istep_;
  ivmphi = meu.ivmphi;
  unit_ = meu.unit_;
  bx = meu.bx;
  istub = meu.istub;
  MEU_COPY: for(int i = 0; i < 1<<MatchEngineUnitBase<VMProjType>::kNBitsBuffer; ++i) {
    #pragma HLS unroll
    stubids[i] = meu.stubids[i];
  }
  /*
  stubids = meu.stubids;
  */
  projfinezadj = meu.projfinezadj;
  tcid = meu.tcid;
  isPSseed = meu.isPSseed;
  zbin = meu.zbin;
  projrinv = meu.projrinv;
  projindex = meu.projindex;
  projbuffer = meu.projbuffer;
  /*
  if(copy_) { // Hack to only copy ptr if stepped (meutmp in MP.h)
  }
  */
  copy_ = false;
}

inline void init(BXType bxin, ProjectionRouterBuffer<BARREL> projbuffer_, const INDEX iproj, int iphi, int unit) {
#pragma HLS inline
#pragma HLS ARRAY_PARTITION variable=stubids complete dim=0
//#pragma HLS dependence variable=stubids inter false
//#pragma HLS resource variable=stubids core=RAM_2P_LUTRAM
  writeindex = iproj;
  readindex = 0;
  idle_ = false;
  done_ = false;
  bx = bxin;
  projbuffer = projbuffer_;
  projindex = projbuffer_.getIndex();
  nstubs=projbuffer.getNStubs();
  ivmphi = projbuffer_.getPhi();
  iphi_ = iphi;
  ptr = 0;
  unit_ = unit;
  //std::cout << "Initializing MEU " << unit_ << std::endl;
  //print();
  //stubmem = stubmem_;
  //std::cout << std::hex << "Initializing iphi=" << ivmphi << "\t" << projbuffer.getProjection() << " Received writeindex=" << writeindex << std::endl;
  //projbuffer.Print();

}

inline bool empty() {
#pragma HLS inline  
  return (readindex==0);

}

inline bool idle() {
#pragma HLS inline  
  //std::cout << std::hex << "iphi=" << ivmphi << (idle_ ? "": " not") << " idle!" << std::endl;
  //if(idle_) std::cout << "MEU " << unit_ << " idle" << std::endl;
  return ptr > readindex ? true : idle_;
  return idle_;
}

inline bool done() {
#pragma HLS inline  
  //std::cout << projbuffer.raw() << " " << " iphi=" << ivmphi << (done_ ? "": " not") << " done!" << std::endl;
  //if(done_) std::cout << "MEU " << unit_ << " done" << std::endl;
  return done_;
}

inline bool ready() {
#pragma HLS inline  
  return ptr < readindex;
  return ready_;
}

inline STUBID* getStubIds() {
#pragma HLS inline  
  return stubids;

}

inline ProjectionRouterBuffer<BARREL>::TCID& getTCID() {
#pragma HLS inline  
  return tcid;
}

inline VMProjection<BARREL>::VMPID getProjindex() {
#pragma HLS inline  
  //std::cout << "projindex=" << projindex << "\tprojid=" << projbuffer.getIndex() << std::endl;
  return projbuffer.getIndex();
}

inline NSTUBS getNStubs() {
#pragma HLS inline  
  return nstubs;
}
inline int getIPhi() {
#pragma HLS inline  
  return iphi_;
}

inline void read(ProjectionRouterBuffer<BARREL>::TCID& trackletid, VMProjection<BARREL>::VMPID& id, STUBID* stubid, NSTUBS& nstub) {
#pragma HLS inline  
  //std::cout << "reading MEU " << projbuffer.raw() << "\tprojid=" << projbuffer.getIndex() << "\t" << "iphi=" << ivmphi << "\treading=" << readindex << "\tmax=" << writeindex << std::endl;
  trackletid = getTCID();
  id = getProjindex();
  stubid = getStubIds();
  nstub = getNStubs();
  idle_ = true;

}

inline void readNext(ProjectionRouterBuffer<BARREL>::TCID& trackletid, VMProjection<BARREL>::VMPID& projid, STUBID& stubid) {
#pragma HLS inline  
  //print();
  trackletid = getTCID();
  projid = getProjindex();
  stubid = stubids[ptr];
  ptr++;
  //idle_ = ptr > readindex ? true : idle_;
  //std::cout << std::hex << "reading MEU " << unit_ << " " << projbuffer.getProjection() << "\tprojid=" << projbuffer.getIndex() << "\tstubid=" << stubid << "\t" << "iphi=" << ivmphi << "\treading=" << readindex << "\tptr=" << ptr << "\tmax=" << writeindex << std::endl;
  //idle_ = ptr >= readindex ? true : idle_;
  //idle_ = ready() ? idle_ : true;

}

#ifndef __SYNTHESIS__
void print() {
  if(empty()) 
    std::cout << "Empty MEU " << unit_ << " projid=" << projbuffer.getIndex() << std::endl;
  else {
    std::cout << "Unread contents in MEU "  << unit_ << " projid=" << projbuffer.getIndex() << " " << projbuffer.getProjection() << std::endl;
    for(int i = ptr; i < readindex; ++i){
      std::cout << std::hex << i << ": " << stubids[i] << (i==ptr ? " <=== ptr" : "" ) << std::endl;
    }
  }
}
#endif

//inline bool step(bool *table, const VMStubMEMemory<VMSMEType,3>* stubmem, ProjectionRouterBuffer<BARREL> *projbuffer) {
inline bool step(bool *table, const VMStubMEMemoryCM<VMSMEType,3,3> &stubmem) {
#pragma HLS inline
//#pragma HLS dependence variable=istub inter WAR true
//#pragma HLS dependence variable=idle_ intra RAW true
    if(idle() || done()) return true;
    copy_ = true;
    //std::cout << "step " << projbuffer.getProjection() << "\t" << "iphi=" << ivmphi << "\treading=" << readindex << "\tmax=" << nstubs << std::endl;

    ////////////////////////////////////////////
    //This seems like where the ME buffer starts
    //Goes outside if(valid) ?
    ////////////////////////////////////////////
    //If the buffer is not empty we have a projection that we need to 
    //process. 

    //Determin if buffere is empty
    //Buffer for projections
    bool buffernotempty=(writeindex!=readindex);
    bool buffernotlarger=(readindex<=writeindex);
 
    // vmproj index
    typename VMProjection<VMPTYPE>::VMPZBIN projzbin;

  // Buffer still has projections to read out
  //If the buffer is not empty we have a projection that we need to 
  //process. 
  if (!done()) {// && buffernotlarger) {
      auto readindextmp = readindex;

      ap_uint<kNBits_MemAddrBinned+2> istubtmp=istub;

      if (istub==0) {

        //Need to read the information about the proj in the buffer
        auto const qdata=projbuffer;
        tcid=qdata.getTCID();
        auto nstub = qdata.getNStubs();
        nstubs=qdata.getNStubs();
        VMProjection<BARREL> data(qdata.getProjection());
        zbin=qdata.getZBin();

        projindex=data.getIndex();
        auto projfinez=data.getFineZ();
        projrinv=data.getRInv();
        isPSseed=data.getIsPSSeed();
        auto projzbin=qdata.getZBin();

        auto second=qdata.hasSecond();

        //Calculate fine z position
        if (second) {
          projfinezadj=projfinez-8;
        } else {
          projfinezadj=projfinez;
        }

        if (nstubs==1) {
          istub=0;
          readindex++;
          done_ = true;
        } else {
          istub++;
        }
      } else {
        //Check if last stub, if so, go to next buffer entry 
        if (istub+1>=nstubs){
          istub=0;
          readindex++;
          done_ = true;
        } else {
          istub++;
        }
      }
      
      //Read stub memory and extract data fields
      //auto const  stubadd=zbin.concat(istubtmp);
      int stubadd=16*(iphi_*8+zbin)+istubtmp;
      //std::cout << "reading istubtmp=" << istubtmp << " stubadd=" << stubadd << std::endl;
      const VMStubMECM<VMSMEType> stubdata=stubmem.read_mem(bx,stubadd);
      //const VMStubMECM<VMSMEType> stubdata=stubmem[unit_].read_mem(bx,stubadd);
      auto stubindex=stubdata.getIndex();
      //std::cout << "stubid=" << stubindex << std::endl;
      auto stubfinez=stubdata.getFineZ();
      auto stubbend=stubdata.getBend();

      //Check if stub z position consistent
      ap_int<5> idz=stubfinez-projfinezadj;
      bool pass;
      if (isPSseed) {
        pass=idz>=-2&&idz<=2;
      } else {
        pass=idz>=-5&&idz<=5;
      }

      //Check if stub bend and proj rinv consistent
      auto const index=projrinv.concat(stubbend);
      if (pass&&table[index]) {
      stubids[readindextmp]=stubindex;
      //std::cout << std::hex << "MEU found stubid=" << stubindex << std::endl;
    } // if(pass&&table[index])
    //print();
    //if(istub==0 && nstubs>0) idle_ = true;
    ready_ = ptr < readindex;
    if(readindex>=nstubs) done_ = true;
    //std::cout << "readindex=" << readindex << "\tnstubs=" << nstubs << "\tdone=" << done() << std::endl;
    //if(readindex>writeindex) done_ = true;
    //if(istub==0 && nstubs>0) return true;
    //if(done_) std::cout << "MEU " << unit_ << "done stepping" << std::endl;
    if(done_) return true;
 
    //-----------------------------------------------------------------------------------------------------------
    //-------------------------------------- MATCH CALCULATION STEPS --------------------------------------------
    //-----------------------------------------------------------------------------------------------------------

    // Use the stub and projection indices to pick up the stub and projection

  } // if(buffernotempty)
  return false;

} // end step

 //protected:
  INDEX writeindex;
  INDEX readindex;
  INDEX ptr;
  NSTUBS nstubs;
  bool idle_;
  bool ready_;
  bool done_;
  bool copy_;
  int istep_;
  int ivmphi;
  int iphi_;
  int unit_;
  BXType bx;
  NSTUBS istub=0;
  STUBID stubids[1<<MatchEngineUnitBase<VMProjType>::kNBitsBuffer];
  ap_int<5> projfinezadj; //FIXME Need replace 5 with const
  ProjectionRouterBuffer<BARREL>::TCID tcid;
  ProjectionRouterBuffer<BARREL>::PRHASSEC isPSseed;
  ProjectionRouterBuffer<BARREL>::VMPZBIN zbin;
  VMProjection<BARREL>::VMPRINV projrinv;
  VMProjection<BARREL>::VMPID projindex;
  //const VMStubMEMemory<VMSMEType,3> *stubmem;
  ProjectionRouterBuffer<BARREL> projbuffer;

}; // end class


#endif
