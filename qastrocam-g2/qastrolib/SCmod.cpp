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


#include "SCmod.hpp"
#include "QCamVesta.hpp"
#include "QCamDC60.hpp"
#include "SettingsBackup.hpp"

#include "dc60_private_ioctls.h"

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <fcntl.h>

extern settingsBackup settings;

SCmod::~SCmod() {
   stopAccumulation();
   leaveLongPoseMode();
}

void SCmod::setLevels(bool polarity) {
   inverted_=polarity;
   if (inverted_) settings.setKey("LX_LEVELS_INVERTED","yes");
   else settings.setKey("LX_LEVELS_INVERTED","no");
}


/* TOUCAM LED */

SCmodTucLed::SCmodTucLed(QCamVesta & cam) : cam_(cam) {
   if(settings.haveKey("LX_LEVELS_INVERTED"))
      inverted_=(strcasecmp(settings.getKey("LX_LEVELS_INVERTED"),"YES")==0);
   else
      inverted_=false;
   stopAccumulation();
}

void SCmodTucLed::enterLongPoseMode() {
   cam_.setExposure(0xFFFF);
}

void SCmodTucLed::leaveLongPoseMode() {
   stopAccumulation();
}

void SCmodTucLed::stopAccumulation() {
   if (inverted_)
      cam_.setLed(25500,0);
   else
      cam_.setLed(0,25500); // switching led ON/OFF for TUC USB
}

void SCmodTucLed::startAccumulation() {
   if (inverted_)
      cam_.setLed(0,25500);
   else
      cam_.setLed(25500,0); // switching led ON/OFF for TUC USB
}

SCmodTucLed::~SCmodTucLed() {
   stopAccumulation();
   leaveLongPoseMode();
}

/* DC60 GPSW */

SCmodDC60::SCmodDC60(QCamDC60 & cam) : cam_(cam) {
   if(settings.haveKey("LX_LEVELS_INVERTED")) {
      inverted_=(strcasecmp(settings.getKey("LX_LEVELS_INVERTED"),"YES")==0);
      if(inverted_)
         cam_.setInverted(true);
      else
         cam_.setInverted(false);
   }
   stopAccumulation();
}

void SCmodDC60::enterLongPoseMode() {
}

void SCmodDC60::leaveLongPoseMode() {
   stopAccumulation();
}

void SCmodDC60::stopAccumulation() {
   //if (inverted_)
      //cam_.setGPSW(true);
   //else
      //cam_.setGPSW(false); // switching GPSW on/off
   cam_.stopIntegration();
}

void SCmodDC60::startAccumulation() {
   //if (inverted_)
      //cam_.setGPSW(false);
   //else
      //cam_.setGPSW(true); // switching GPSW on/off
   cam_.startIntegration();
}

SCmodDC60::~SCmodDC60() {
   stopAccumulation();
   leaveLongPoseMode();
}

/* SERIAL PORT */

SCmodSerialPort::SCmodSerialPort() {
   if(settings.haveKey("LX_LEVELS_INVERTED"))
      inverted_=(strcasecmp(settings.getKey("LX_LEVELS_INVERTED"),"YES")==0);
   else
      inverted_=false;

   if(settings.haveKey("LX_DEVICE"))
      device=settings.getKey("LX_DEVICE");
   else
      device="/dev/ttyS0";

   device_=open(device.c_str(),O_WRONLY);
   if (device_<0) {
      perror(device.c_str());
   }
   stopAccumulation();
}

SCmodSerialPort::~SCmodSerialPort() {
   stopAccumulation();
   leaveLongPoseMode();
   close(device_);
}

void SCmodSerialPort::enterLongPoseMode() {
   //nothing to do
}

void SCmodSerialPort::leaveLongPoseMode() {
   stopAccumulation();
}

void SCmodSerialPort::stopAccumulation() {
   int flag;
   int function;

   if (inverted_)
      function=TIOCMBIC;
   else
      function=TIOCMBIS;
   // set preamp on
   flag=TIOCM_DTR;
   if (ioctl(device_,function,&flag)) {
      perror("set dtr");
   }
   usleep(800);
   // unblock exposure
   flag=TIOCM_RTS;
   if (ioctl(device_,function,&flag)) {
      perror("set rts");
   }
}

void SCmodSerialPort::startAccumulation() {
   int flag;
   int function;

   if (inverted_)
      function=TIOCMBIS;
   else
      function=TIOCMBIC;
   // block exposure
   flag=TIOCM_RTS;
   if (ioctl(device_,function,&flag)) {
      perror("set rts");
   }
   // switch preamp off
   flag=TIOCM_DTR;
   if (ioctl(device_,function,&flag)) {
      perror("set dtr");
   }
}
