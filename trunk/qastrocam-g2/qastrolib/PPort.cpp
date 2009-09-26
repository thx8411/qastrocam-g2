#include "PPort.hpp"

#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/lp.h>
#include <linux/ppdev.h>

#include <iostream>

PPort* PPort::instance_=NULL;

PPort* PPort::instance() {
   if(instance_==NULL)
      instance_=new PPort();
   return(instance_);
}

void PPort::destroy() {
   if(instance_!=NULL)
      delete instance_;
   instance_=NULL;
}

PPort::PPort() {
   pportTableSize=0;
}

PPort::~PPort() {
   unsigned char buff=0x00;
   int i;

   for(i=0;i<pportTableSize;i++) {
      if(pportTable[i].type==LP_TYPE)
         write(pportTable[i].fd,&buff,1);
      else if (pportTable[i].type==PPDEV_TYPE)
         ioctl(pportTable[i].fd, PPWDATA, &buff);
      close(pportTable[i].fd);
   }
}

int PPort::pportFind(const char* name) {
   int i=0;
   while((i<pportTableSize)&&(pportTable[i].name!=name))
      i++;
   if(i!=pportTableSize)
      return(i);
   else
      return(-1);
}

int PPort::getAccess(const char* device) {
   unsigned char buff=0x00;
   int index;
   index=pportFind(device);

   //cout << "ppdev " << index << endl;

   if(index!=-1)
      return(index);
   else {
      if(pportTableSize==PPORT_TABLE_SIZE-1) {
            cerr << "no more space left in the pport table" << endl;
            return(-1);
         }

      pportTable[pportTableSize].fd=open(device,O_WRONLY);
      if(pportTable[pportTableSize].fd<0) {
         cerr << "unable to open device " << device << endl;
         return(-1);
      }

      pportTable[pportTableSize].name=device;
      pportTable[pportTableSize].data=0x00;

      if (ioctl(pportTable[pportTableSize].fd, PPCLAIM, 0) != 0) {
         cout << "Not a ppdev device, using standard lp access" << endl;
         ioctl(pportTable[pportTableSize].fd, LPRESET);
         write(pportTable[pportTableSize].fd,&buff,1);
         pportTable[pportTableSize].type=LP_TYPE;
         pportTableSize++;
         return(pportTableSize-1);
      }

      int outputmode=0;
      if (ioctl(pportTable[pportTableSize].fd, PPDATADIR, &outputmode) != 0) {
         cout << "Unable to set the port direction, using it as a standard lp port" << endl;
         ioctl(pportTable[pportTableSize].fd, LPRESET);
         write(pportTable[pportTableSize].fd,&buff,1);
         pportTable[pportTableSize].type=LP_TYPE;
         pportTableSize++;
         return(pportTableSize-1);
      }

      pportTable[pportTableSize].type=PPDEV_TYPE;
      pportTableSize++;
      return(pportTableSize-1);
   }
}

bool  PPort::setBit(int bit,bool value, int entry) {
   int res;
   unsigned char data;

   //cout << "setting bit " << bit << " to " << value << endl;

   if((entry<0)||(entry>=pportTableSize)) {
      cerr << "wrong entry number for pport " << endl;
      return(false); 
   }

   data=0x01<<bit;
   if(value)
      pportTable[entry].data|=data;
   else
      pportTable[entry].data&=~data;

   data=pportTable[entry].data;
   if(pportTable[entry].type==LP_TYPE)
      res=write(pportTable[entry].fd,&data,1);
   else if(pportTable[entry].type==PPDEV_TYPE) {
      res=ioctl(pportTable[entry].fd, PPWDATA, &data);
      if(res==0) res=1;
   } else {
      cerr << "unsupported device type" << endl;
      return(false);
   }

   usleep(1);

   return(res==1);
}

