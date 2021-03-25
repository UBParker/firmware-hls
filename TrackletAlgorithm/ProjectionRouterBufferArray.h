#ifndef PRBUFFERARRAY_HH
#define PRBUFFERARRAY_HH

#include "ProjectionRouterBuffer.h"

template<int kNBitsBuffer,int AllProjectionType> class ProjectionRouterBufferArray {
public:
  inline ProjectionRouterBuffer<BARREL,AllProjectionType> read() {
#pragma HLS inline
    return projbuffer_[readptr_++];
  }

  inline void addProjection(ProjectionRouterBuffer<BARREL,AllProjectionType> &proj) {
#pragma HLS inline
    projbuffer_[writeptr_++] = proj;
  }

  inline bool empty() { 
    return readptr_ == writeptr_;
  }

  bool nearFull() {
    ap_uint<kNBitsBuffer> writeptrnext(writeptr_+1);
    ap_uint<kNBitsBuffer> writeptrnextnext(writeptr_+2);
    return writeptrnext==readptr_ || writeptrnextnext==readptr_;
  }


  inline void reset() {
#pragma HLS inline
    readptr_ = 0;
    writeptr_ = 0;
  }

  ProjectionRouterBufferArray() {
    reset();
  }

private:
  ap_uint<kNBitsBuffer> readptr_ = 0;
  ap_uint<kNBitsBuffer> writeptr_ = 0;
  ProjectionRouterBuffer<BARREL,AllProjectionType> projbuffer_[1<<kNBitsBuffer];

};

#endif