
#include <iostream>
#include <string.h>

#include "SCmodParPortPPdev.hpp"
#include "SettingsBackup.hpp"

extern settingsBackup settings;

SCmodParPortPPdev::SCmodParPortPPdev() {

   if(settings.haveKey("LX_DEVICE"))
      device=settings.getKey("LX_DEVICE");
   else
      device="/dev/parport0";

   if(settings.haveKey("LX_LEVELS_INVERTED")) {
      inverted=(strcasecmp(settings.getKey("LX_LEVELS_INVERTED"),"YES")==0);
   } else {
      inverted=false;
   }

   paralPort=PPort::instance();

   portEntry=paralPort->getAccess(device.c_str());
   if(portEntry==-1) {
      cerr << "unable to get access to " << device << endl;
   }

   leaveLongPoseMode();
   stopAccumulation();
}

SCmodParPortPPdev::~SCmodParPortPPdev() {
   paralPort->destroy();
}

void SCmodParPortPPdev::enterLongPoseMode() {
   if(inverted)
      paralPort->setBit(shutter,false,portEntry);
   else
      paralPort->setBit(shutter,true,portEntry);
}

void SCmodParPortPPdev::leaveLongPoseMode() {
   if(inverted)
      paralPort->setBit(shutter,true,portEntry);
   else
      paralPort->setBit(shutter,false,portEntry);
}

void SCmodParPortPPdev::stopAccumulation() {
   if(inverted)
      paralPort->setBit(preamp,false,portEntry);
   else
      paralPort->setBit(preamp,true,portEntry);

   usleep(800);

   if(inverted) {
      paralPort->setBit(evenLinesTransfer,false,portEntry);
      paralPort->setBit(oddLinesTransfer,false,portEntry);
   } else {
      paralPort->setBit(evenLinesTransfer,true,portEntry);
      paralPort->setBit(oddLinesTransfer,true,portEntry);
   }
}

void SCmodParPortPPdev::startAccumulation() {
   if(inverted) {
      paralPort->setBit(evenLinesTransfer,true,portEntry);
      paralPort->setBit(oddLinesTransfer,true,portEntry);
      paralPort->setBit(preamp,true,portEntry);
   } else {
      paralPort->setBit(evenLinesTransfer,false,portEntry);
      paralPort->setBit(oddLinesTransfer,false,portEntry);
      paralPort->setBit(preamp,false,portEntry);
   }
}
