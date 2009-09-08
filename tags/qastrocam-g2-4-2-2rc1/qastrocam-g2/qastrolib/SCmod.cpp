#include "SCmod.hpp"
#include "QCamVesta.hpp"

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <fcntl.h>

SCmodTucLed::SCmodTucLed(QCamVesta & cam) : cam_(cam) {
   stopAccumulation();
}

void SCmodTucLed::enterLongPoseMode() {
   cam_.setExposure(0xFFFF);
}

void SCmodTucLed::leaveLongPoseMode() {
   stopAccumulation();
}

void SCmodTucLed::stopAccumulation() {
   cam_.setLed(0,1000); // switching led OFF for TUC USB
}

void SCmodTucLed::startAccumulation() {
   cam_.setLed(1000,0); // switching led ON  for TUC USB
}




SCmodSerialPort::SCmodSerialPort(const char * device) {
   device_=open(device,O_WRONLY);
   if (device_<0) {
      perror(device);
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

	int err;

   // set preamp on
   flag=TIOCM_DTR;
   if (ioctl(device_,TIOCMBIC,&flag)) {
      perror("set dtr");
   }
   usleep(800);
   // unblock exposure
   flag=TIOCM_RTS;
   if (ioctl(device_,TIOCMBIC,&flag)) {
      perror("set rts");
   }
}

void SCmodSerialPort::startAccumulation() {
   int flag;

   // block exposure
   flag=TIOCM_RTS;
   if (ioctl(device_,TIOCMBIS,&flag)) {
      perror("set rts");
   }   

   // switch preamp off
   flag=TIOCM_DTR;
   if (ioctl(device_,TIOCMBIS,&flag)) {
      perror("set dtr");
   }
}
