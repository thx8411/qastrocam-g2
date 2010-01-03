#ifndef _bayer_hpp_
#define _bayer_hpp_
#include "QCamFrame.hpp"

// vesta raw to rgb conversion
void raw2rgb(unsigned char * dest,const unsigned char * const data,
               int w, int h,int mode);

// vesta raw to yuv420p conversion
void raw2yuv420p(unsigned char* Y, unsigned char* U, unsigned char* V, unsigned char* input, const int w, const int h, int mode);

#endif
