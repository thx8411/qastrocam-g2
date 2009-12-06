#include "QTelescopeFifo.moc"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>

using namespace std;

QTelescopeFifo::QTelescopeFifo(const char * deviceName) :
   QTelescope() {
   descriptor_=open(deviceName,O_RDWR|O_NOCTTY);
   if (descriptor_==-1) {
      perror(deviceName);
   }
}
