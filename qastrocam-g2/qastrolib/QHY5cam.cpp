/******************************************************************
Qastrocam-g2
Copyright (C) 2010 Blaise-Florentin Collin
Thanks to Geoffrey Hauser

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

// QHY5 native driver access
// singleton class

#include <stdlib.h>
#include <usb.h>
#include <iostream>

#include <qmessagebox.h>

#include "QHY5cam.hpp"

#define STORE_WORD_BE(var, val)	*(var)=((val)>>8)&0xff; *((var)+1)=(val)&0xff

//
// singleton stuff
//

// static members init
QHY5cam* QHY5cam::instance_=NULL;
bool QHY5cam::feature_used[2]={false,false};

QHY5cam* QHY5cam::instance(int feature) {
   if(feature>=2)
      return(NULL);
   if(instance_==NULL) {
      instance_=new QHY5cam();
      feature_used[feature]=true;
      return(instance_);
   }
   if(feature_used[feature])
      return(NULL);
   return(instance_);
}

void QHY5cam::destroy(int feature) {
   feature_used[feature]=false;
   if(!(feature_used[QHY_IMAGER] || feature_used[QHY_GUIDER])) {
      delete instance_;
      instance_=NULL;
   }
}

//
// functions
//

// class functions
int QHY5cam::shoot(int duration) {
   int val,index;
   char buffer[2]={0x00,0x00};

   index= duration >> 16;
   val= duration & 0xffff;

   return(usb_control_msg(dev,0xc2,0x12,val, index, buffer, 2, 5000));
}

int QHY5cam::read(void* image) {
   return(usb_bulk_read(dev,0x82,(char*)image,size_,20000));
}

int QHY5cam::move(int direction, int duration) {
   //
   return(0);
}

int  QHY5cam::configure(int xpos, int ypos, int w, int h, int gain, int* rw, int* rh) {
   char registers[19];
   int offset,index,value,res;

   xpos_=xpos;
   ypos_=ypos;
   width_=w;
   height_=h-(h%4);
   *rw=width_;
   *rh=height_;
   gain_=gain*0x6ff/100;
   size_=1558*(height_+26);
   offset=(1048-height_)/2;
   index=(1558*(height_+26))>>16;
   value=(1558*(height_+26))&0xffff;
   STORE_WORD_BE(registers+0,gain_);
   STORE_WORD_BE(registers+2,gain_);
   STORE_WORD_BE(registers+4,gain_);
   STORE_WORD_BE(registers+6,gain_);
   STORE_WORD_BE(registers+8,offset);
   STORE_WORD_BE(registers+10,0);
   STORE_WORD_BE(registers+12,height_-1);
   STORE_WORD_BE(registers+14,0x0521);
   STORE_WORD_BE(registers+16,height_+25);
   registers[18]=0xcc;
   res=usb_control_msg(dev,0x42,0x13,value,index,registers,19,5000);
   usb_control_msg(dev,0x42,0x14,0x31a5,0,registers,0,5000);
   usb_control_msg(dev,0x42,0x16,0,0,registers,0,5000);

   return(0);
}

// is the cam plugged ?
bool QHY5cam::plugged() {
   struct usb_bus* bus;
   struct usb_device* device;

   // update usb datas
   usb_init();
   usb_find_busses();
   usb_find_devices();

   for(bus = usb_busses; bus; bus = bus->next) {
      for(device = bus->devices; device; device=device->next) {
         if((device->descriptor.idVendor == 0x16c0)&&(device->descriptor.idProduct == 0x296d))
            return(true);
      }
   }
   return(false);
}

QHY5cam::QHY5cam() {
   int temp1,temp2;
   struct usb_bus* bus;
   struct usb_device* device;

   // init
   dev=NULL;

   // update usb datas
   usb_init();
   usb_find_busses();
   usb_find_devices();

   // get the usb device handle
   for(bus = usb_busses; bus; bus = bus->next) {
      for(device = bus->devices; device; device=device->next) {
         if((device->descriptor.idVendor == 0x16c0)&&(device->descriptor.idProduct == 0x296d)) {
            dev=usb_open(device);
            if(dev==NULL) {
               QMessageBox::information(0,"Qastrocam-g2","Unable to reach the QHY5 imager\nLeaving...");
               exit(1);
            }
            int res;
            res=usb_set_configuration(dev,1);
            res=usb_claim_interface(dev,0);
            res=usb_set_altinterface(dev,0);
         }
      }
   }

   // init config
   configure(0,0,1280,1024,10,&temp1,&temp2);
}

QHY5cam::~QHY5cam() {
   // close usb device
   usb_close(dev);
}
