#ifndef _SCmodParPortPPdev_hpp_
#define _SCmodParPortPPdev_hpp_

#include "SCmod.hpp"
#include "qobject.h"

#include <string>

using namespace std;

class SCmodParPortPPdev : public SCmod {
public:
   SCmodParPortPPdev();
   virtual ~SCmodParPortPPdev();
   void enterLongPoseMode();
   void leaveLongPoseMode();
   void stopAccumulation();
   void startAccumulation();
private:
   enum pportBit {evenLinesTransferOn=0,
                  oddLinesTransferOn=1,
                  preampOn=2,
                  shutterOn=3};
   void sendPportCmd();
   void activatePPort();
   int ppdev_fd;
   int data_out;
   string device;
};

#endif
