/******************************************************************
Qastrocam-g2
Copyright (C) 2010 Blaise-Florentin Collin
Thanks to Geoffrey Hauser and Clive

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
// Uses Tom's firmware
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
   // claim device part
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

// release the device part
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

// resets the cam
int QHY5cam::stop() {
   char data=0x00;
   return(usb_bulk_write(dev,0,&data,1,1000));
}

// start picture shoot
int QHY5cam::shoot(int duration) {
   int val,index,ret;
   char buffer[11]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

   index= duration >> 16;
   val= duration & 0xffff;

   ret=usb_control_msg(dev,0xc2,0x12,val, index, buffer, 10, 500);

   int tmp=buffer[0];

   return(ret);
}

// read the picture
int QHY5cam::read(char* image) {
   int res,line,row,offset;
   offset=0;

   if(image==NULL) return(-1);

   res=usb_bulk_read(dev,0x82,image_,size_,/*100*/0);
   if(res==size_) {
      for(line=0;line<height_;line++) {
         for(row=0;row<width_;row++) {
            //
            // to be fixed
            image[offset]=image_[1558*line+20+row+xpos_];
            //
            offset++;
         }
      }
   } //else
   //   stop();
   return(res==size_);
}


// set the autoguide port corrections
// direction : QHY_NORTH to QHY_WEST
// duration in ms, 0 to cancel
// one move per call
int QHY5cam::move(int direction, int duration) {
   unsigned int ret;
   int pulses[2]={-1,-1};

   if((duration==0)||(direction==QHY_NONE)) {
      switch(direction) {
         case QHY_NORTH :
         case QHY_SOUTH :
            direction=0x22;
            break;
         case QHY_EAST :
         case QHY_WEST :
            direction=0x21;
            break;
         case QHY_NONE :
         default :
            direction=0x18;
      }
      return(usb_control_msg(dev,0xc2,direction,0,0,(char*)&ret,sizeof(&ret),500));
   }
   // apply moves
   switch(direction) {
      case QHY_NORTH :
         direction=0x20;
         pulses[1]=duration;
         break;
      case QHY_SOUTH :
         direction=0x40;
         pulses[1]=duration;
         break;
      case QHY_EAST :
         direction=0x10;
         pulses[0]=duration;
         break;
      case QHY_WEST :
         direction=0x80;
         pulses[0]=duration;
         break;
   }
   return(usb_control_msg(dev,0x42,0x10,0,direction,(char*)pulses,sizeof(pulses),500));
}

int QHY5cam::move(int direction) {
   //
   return(0);
   //
}

// configure the cam
int  QHY5cam::configure(int xpos, int ypos, int w, int h, int gain, int* rw=NULL, int* rh=NULL) {
   char registers[19];
   int offset,index,value,res;
   const int setgain[74]={0x000,0x004,0x005,0x006,0x007,0x008,0x009,0x00A,0x00B,
                   0x00C,0x00D,0x00E,0x00F,0x010,0x011,0x012,0x013,0x014,
                   0x015,0x016,0x017,0x018,0x019,0x01A,0x01B,0x01C,0x01D,
                   0x01E,0x01F,0x051,0x052,0x053,0x054,0x055,0x056,0x057,
                   0x058,0x059,0x05A,0x05B,0x05C,0x05D,0x05E,0x05F,0x6CE,
                   0x6CF,0x6D0,0x6D1,0x6D2,0x6D3,0x6D4,0x6D5,0x6D6,0x6D7,
                   0x6D8,0x6D9,0x6DA,0x6DB,0x6DC,0x6DD,0x6DE,0x6DF,0x6E0,
                   0x6E1,0x6E2,0x6E3,0x6E4,0x6E5,0x6E6,0x6E7,0x6FC,0x6FD,0x6FE,0x6FF};

   // test values
   if(w>1280) w=1280;
   if(w<1) w=1;
   if(h>1024) h=1024;
   if(h<1) h=1;
   if(xpos<0) xpos=0;
   if(ypos<1) ypos=0;
   if((xpos+w)>1280) w=1280-xpos;
   if((ypos+h)>1024) h=1024-ypos;
   if(gain<0) gain=0;
   if(gain>73) gain=73;
   // setting registers
   xpos_=xpos;
   ypos_=ypos;
   width_=w;
   height_=h-(h%4);
   if(rw) *rw=width_;
   if(rh) *rh=height_;
   gain_=setgain[gain];
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
   res=usb_control_msg(dev,0x42,0x13,value,index,registers,19,500);
   usb_control_msg(dev,0x42,0x14,0x31a5,0,registers,0,500);
   usb_control_msg(dev,0x42,0x16,0,0,registers,0,500);
   // alloc mem
   free(image_);
   image_=(char*)malloc(size_);

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
   // look for the device
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
   image_=NULL;

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
   // free buffer
   free(image_);
}
