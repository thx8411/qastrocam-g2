#ifndef _bayer_hpp_
#define _bayer_hpp_
#include "QCamFrame.hpp"

// vesta raw to rgb conversion
void raw2rgb(unsigned char * dest,const unsigned char * const data,
               int w, int h,int mode);
#endif
