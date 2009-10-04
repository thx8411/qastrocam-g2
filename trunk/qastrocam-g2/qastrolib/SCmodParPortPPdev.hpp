#ifndef _SCmodParPortPPdev_hpp_
#define _SCmodParPortPPdev_hpp_

#include "SCmod.hpp"
#include "PPort.hpp"

#include <string>

using namespace std;

// driving lxmode using part port
class SCmodParPortPPdev : public SCmod {
public:
   SCmodParPortPPdev();
   virtual ~SCmodParPortPPdev();
   void enterLongPoseMode();
   void leaveLongPoseMode();
   void stopAccumulation();
   void startAccumulation();
private:
   // lists used par port bits
   enum pportBit {evenLinesTransfer=0,
                  oddLinesTransfer=1,
                  preamp=2,
                  shutter=3};
   // entry i PPdev table
   int portEntry;
   // PPdev object
   PPort* paralPort;
   // are logical level on the port inverted ?
   bool inverted;
   // port device name
   string device;
};

#endif
