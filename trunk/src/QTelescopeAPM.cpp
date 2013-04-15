/******************************************************************
Qastrocam
Copyright (C) 2003-2009   Franck Sicard
Qastrocam-g2
Copyright (C) 2009   Blaise-Florentin Collin

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License v2
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
MA  02110-1301, USA.
*******************************************************************/

#include <Qt/qmessagebox.h>

#include "QTelescopeAPM.hpp"

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
      QMessageBox::information(0,"Qastrocam-g2","Unable to reach the telescope device\nThe mount won't move...");
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
   return(0);
}

bool QTelescopeAPM::setTracking(bool activated) {
   // always tracking ?
   return activated;
}
