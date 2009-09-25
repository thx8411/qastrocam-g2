#include "PPort.hpp"

#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
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
   int i;
   for(i=0;i<pportTableSize;i++)
      close(pportTable[i].fd);
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
   int index;
   index=pportFind(device);

   if(index!=-1)
      return(pportTable[index].fd);
   else {
      if(pportTableSize=PPORT_TABLE_SIZE-1) {
            cerr << "no more space left in the pport table" << endl;
            return(-1);
         }

      pportTable[pportTableSize].fd=open(device,O_WRONLY);
      if(pportTable[pportTableSize].fd<0) {
         cerr << "unable to open device " << device << endl;
         return(-1);
      }

      pportTable[pportTableSize].name=device;
      pportTable[pportTableSize].data=0;

      if (ioctl(pportTable[pportTableSize].fd, PPCLAIM, 0) != 0) {
         cout << "Not a ppdev device, using standard lp access" << endl;
         pportTable[pportTableSize].type=LP_TYPE;
         pportTableSize++;
         return(pportTableSize-1);
      }

      int outputmode=0;
      if (ioctl(pportTable[pportTableSize].fd, PPDATADIR, &outputmode) != 0) {
         cout << "Unable to set the port direction, using it as a standard lp port" << endl;
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

   if((entry<0)||(entry>=pportTableSize)) {
      cerr << "wrong entry number for pport " << endl;
      return(false); 
   }

   if(value)
      pportTable[entry].data|=1<<bit;
   else
      pportTable[entry].data&=0xFE<<bit;
   
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

   return(res==1);
}

