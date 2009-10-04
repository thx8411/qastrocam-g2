
#include <iostream>
#include <string.h>

#include "SCmodParPortPPdev.hpp"
#include "SettingsBackup.hpp"

extern settingsBackup settings;

SCmodParPortPPdev::SCmodParPortPPdev() {
   // looking for a known device in settings file
   if(settings.haveKey("LX_DEVICE"))
      device=settings.getKey("LX_DEVICE");
   else
      // else default device is parport0
      device="/dev/parport0";
   // checks if levels are inverted or not in the settings file
   if(settings.haveKey("LX_LEVELS_INVERTED")) {
      inverted=(strcasecmp(settings.getKey("LX_LEVELS_INVERTED"),"YES")==0);
   } else {
      // else no
      inverted=false;
   }
   // get the instance of PPdev or create one (singleton)
   paralPort=PPort::instance();
   // ask accesstothe port
   portEntry=paralPort->getAccess(device.c_str());
   if(portEntry==-1) {
      cerr << "unable to get access to " << device << endl;
   }
   // default init : lx mode disabled
   leaveLongPoseMode();
   stopAccumulation();
}

SCmodParPortPPdev::~SCmodParPortPPdev() {
   // destroy the instance if needed
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

   // awake the amp before reading
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
