#ifndef _SCmodParPortPPdev_hpp_
#define _SCmodParPortPPdev_hpp_

#include "SCmod.hpp"
#include "PPort.hpp"

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
   enum pportBit {evenLinesTransfer=0,
                  oddLinesTransfer=1,
                  preamp=2,
                  shutter=3};
   int portEntry;
   PPort* paralPort;
   bool inverted;
   string device;
};

#endif
