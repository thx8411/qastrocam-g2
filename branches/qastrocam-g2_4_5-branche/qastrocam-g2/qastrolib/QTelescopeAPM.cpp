#include "QTelescopeAPM.moc"

#include <iostream>
#include <string.h>

#include "SettingsBackup.hpp"

using namespace std;

extern settingsBackup settings;

QTelescopeAPM::QTelescopeAPM(const char * pport) : QTelescope() {
   portName=pport;
   // get an instance of the port object (singleton)
   paralPort=PPort::instance();
   // get access to the port
   portEntry=paralPort->getAccess(portName);
   if(portEntry==-1) {
      cerr << "unable to get access to " << portName << endl;
   }
   // are levels inverted in settings file ?
   if(settings.haveKey("TS_LEVELS_INVERTED")&&(strcasecmp(settings.getKey("TS_LEVELS_INVERTED"),"yes")==0)) {
      go=false;
      stop=true;
   } else {
      go=true;
      stop=false;
   }

   stopE();
   stopW();
   stopN();
   stopS();
}

QTelescopeAPM::~QTelescopeAPM() {
   paralPort->destroy();
}

void QTelescopeAPM::goE(float shift) {
   stopW();
   paralPort->setBit(EastBit,go,portEntry);
}

void QTelescopeAPM::goW(float shift) {
   stopE();
   paralPort->setBit(WestBit,go,portEntry);
}

void QTelescopeAPM::goS(float shift) {
   stopN();
   paralPort->setBit(SouthBit,go,portEntry);
}

void QTelescopeAPM::goN(float shift) {
   stopS();
   paralPort->setBit(NorthBit,go,portEntry);
}

void QTelescopeAPM::stopE() {
   paralPort->setBit(EastBit,stop,portEntry);
}

void QTelescopeAPM::stopN() {
   paralPort->setBit(NorthBit,stop,portEntry);
}

void QTelescopeAPM::stopW() {
   paralPort->setBit(WestBit,stop,portEntry);
}

void QTelescopeAPM::stopS() {
   paralPort->setBit(SouthBit,stop,portEntry);
}

double QTelescopeAPM::setSpeed(double speed) {
   return 2.0/3;
}

bool QTelescopeAPM::setTracking(bool activated) {
   // always tracking ?
   return activated;
}
