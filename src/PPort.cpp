/******************************************************************
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


#include "PPort.hpp"

#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/lp.h>
#include <linux/ppdev.h>

#include <iostream>

// static members init
PPort* PPort::instance_=NULL;
int PPort::clientNumber=0;

// if pport does not exist, create one
PPort* PPort::instance() {
   clientNumber++;
   if(instance_==NULL)
      instance_=new PPort();
   return(instance_);
}

// destroy the port if needed
void PPort::destroy() {
   clientNumber--;
   if(clientNumber==0) {
      if(instance_!=NULL)
         delete instance_;
      instance_=NULL;
   }
}

PPort::PPort() {
   pportTableSize=0;
}

PPort::~PPort() {
   ssize_t tmp;
   unsigned char buff=0x00;
   int i;

   // clear all the entries
   for(i=0;i<pportTableSize;i++) {
      if(pportTable[i].type==LP_TYPE)
         tmp=write(pportTable[i].fd,&buff,1);
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
   ssize_t tmp;
   unsigned char buff=0x00;
   int index;

   // looks for the port if opened
   index=pportFind(device);

   // returns the port entry or creat one
   if(index!=-1)
      return(index);
   else {
      // check space left in table
      if(pportTableSize==PPORT_TABLE_SIZE-1) {
            cerr << "no more space left in the pport table" << endl;
            return(-1);
         }
      // open the port
      pportTable[pportTableSize].fd=open(device,O_WRONLY);
      if(pportTable[pportTableSize].fd<0) {
         cerr << "unable to open device " << device << endl;
         return(-1);
      }
      // fills the entry
      pportTable[pportTableSize].name=device;
      pportTable[pportTableSize].data=0x00;
      // claims the port, if fails, assuming this is an "lp" port
      if (ioctl(pportTable[pportTableSize].fd, PPCLAIM, 0) != 0) {
         cout << "Not a ppdev device, using standard lp access" << endl;
         ioctl(pportTable[pportTableSize].fd, LPRESET);
         tmp=write(pportTable[pportTableSize].fd,&buff,1);
         pportTable[pportTableSize].type=LP_TYPE;
         pportTableSize++;
         return(pportTableSize-1);
      }
      // sets port direction, if fails, assuming this is an "lp" port
      int outputmode=0;
      if (ioctl(pportTable[pportTableSize].fd, PPDATADIR, &outputmode) != 0) {
         cout << "Unable to set the port direction, using it as a standard lp port" << endl;
         ioctl(pportTable[pportTableSize].fd, LPRESET);
         tmp=write(pportTable[pportTableSize].fd,&buff,1);
         pportTable[pportTableSize].type=LP_TYPE;
         pportTableSize++;
         return(pportTableSize-1);
      }
      // everything ok, it's a "parport" device
      pportTable[pportTableSize].type=PPDEV_TYPE;
      pportTableSize++;
      return(pportTableSize-1);
   }
}

bool  PPort::setBit(int bit,bool value, int entry) {
   int res;
   unsigned char data;

   // tests the entry
   if((entry<0)||(entry>=pportTableSize)) {
      cerr << "wrong entry number for pport " << endl;
      return(false);
   }

   // fixing new bit value
   data=0x01<<bit;
   if(value)
      pportTable[entry].data|=data;
   else
      pportTable[entry].data&=~data;
   // send the new byte to the port
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

   // minimum delay for lpt standard between to datas
   usleep(1);
   return(res==1);
}
