#include "SCmod.hpp"
#include "QCamVesta.hpp"
#include "SettingsBackup.hpp"

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
      cam_.setLed(1000,0);
   else 
      cam_.setLed(0,1000); // switching led ON/OFF for TUC USB
}

void SCmodTucLed::startAccumulation() {
   if (inverted_) 
      cam_.setLed(0,1000);
   else 
      cam_.setLed(1000,0); // switching led ON  for TUC USB
}




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
