#include "QTelescopeAPM.moc"

#include "PPort.hpp"
#include <iostream>

using namespace std;

QTelescopeAPM::QTelescopeAPM(PPort * pport) :
   QTelescope() {
   paralPort=pport;
   if (!paralPort->registerBit(this,EastBit)) { cerr << "cant' register bit "<<EastBit<<" on // port\n";}
   if (!paralPort->registerBit(this,WestBit)) { cerr << "cant' register bit "<<WestBit<<" on // port\n";}
   if (!paralPort->registerBit(this,NorthBit)) { cerr << "cant' register bit "<<NorthBit<<" on // port\n";}
   if (!paralPort->registerBit(this,SouthBit)) { cerr << "cant' register bit "<<SouthBit<<" on // port\n";}
}

void QTelescopeAPM::goE(float shift) {
   paralPort->setBit(this,EastBit,true);
   paralPort->setBit(this,WestBit,false);
   paralPort->commit();
}

void QTelescopeAPM::goW(float shift) {
   paralPort->setBit(this,EastBit,false);
   paralPort->setBit(this,WestBit,true);
   paralPort->commit();
}

void QTelescopeAPM::goS(float shift) {
   paralPort->setBit(this,NorthBit,false);
   paralPort->setBit(this,SouthBit,true);
   paralPort->commit();
}

void QTelescopeAPM::goN(float shift) {
   paralPort->setBit(this,NorthBit,true);
   paralPort->setBit(this,SouthBit,false);
   paralPort->commit();
}

void QTelescopeAPM::stopE() {
   paralPort->setBit(this,EastBit,false);
   paralPort->commit();
}

void QTelescopeAPM::stopN() {
   paralPort->setBit(this,NorthBit,false);
   paralPort->commit();
}

void QTelescopeAPM::stopW() {
   paralPort->setBit(this,WestBit,false);
   paralPort->commit();
}

void QTelescopeAPM::stopS() {
   paralPort->setBit(this,SouthBit,false);
   paralPort->commit();
}

double QTelescopeAPM::setSpeed(double speed) {
   return 2.0/3;
}

bool QTelescopeAPM::setTracking(bool activated) {
   /// always tracking ?
   return activated;
}
