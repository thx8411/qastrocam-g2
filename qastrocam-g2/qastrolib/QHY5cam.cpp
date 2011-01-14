/******************************************************************
Qastrocam-g2
Copyright (C) 2010 Blaise-Florentin Collin
Thanks to Geoffrey Hausheer and Clive

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

// Tom's firmware only supports timed moves ont the ST4 port.
// For permanent moves, this driver use a periodic thread, refreshing
// the timed moves until stop

#include <sys/time.h>
#include <stdlib.h>
#include <usb.h>
#include <iostream>

#include <qmessagebox.h>

#include "yuv.hpp"

#include "QHY5cam.hpp"

#define GAIN_SCALE_SIZE	82

#define STORE_WORD_BE(var, val)	*(var)=((val)>>8)&0xff; *((var)+1)=(val)&0xff

//
// singleton stuff
//

// static members init
QHY5cam* QHY5cam::instance_=NULL;
bool QHY5cam::feature_used[2]={false,false};

// get the class instance for a feature. Create the singleton if needed
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
   if(!(feature_used[QHY5_IMAGER] || feature_used[QHY5_GUIDER])) {
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
   int res;

   pthread_mutex_lock(&usbMutex);
   res=usb_bulk_write(dev,0,&data,1,1000);
   pthread_mutex_unlock(&usbMutex);
   return(res);
}

// start picture shoot (duration in ms)
// mode : true -> short exposure
int QHY5cam::shoot(int duration, bool mode) {
   int val,index,ret;

   index= duration >> 16;
   val= duration & 0xffff;

   pthread_mutex_lock(&usbMutex);
   ret=usb_control_msg(dev,0xc2,0x12,val, index, NULL, 0, 0);
   pthread_mutex_unlock(&usbMutex);

   if(mode) {
      usleep(duration*1000);

      pthread_mutex_lock(&exposureMutex);
      pthread_mutex_lock(&usbMutex);
      ret=usb_bulk_read(dev,0x82,(char*)image_,size_,0);
      frameAvailable=(ret==size_);
      pthread_mutex_unlock(&usbMutex);
      pthread_mutex_unlock(&exposureMutex);
   }

   return(ret);
}

// read the picture
// image buffer size : x*y*1 byte
int QHY5cam::read(unsigned char* image, bool mode, bool denoise) {
   int ret,line,row,offset;
   int temp;
   offset=0;

   if(image==NULL) return(-1);

   pthread_mutex_lock(&exposureMutex);
   if(!mode) {
      pthread_mutex_lock(&usbMutex);
      ret=usb_bulk_read(dev,0x82,(char*)image_,size_,0);
      pthread_mutex_unlock(&usbMutex);
      frameAvailable=(ret==size_);
   }

   // denoise filtering stuff
   if(denoise) {
      memset(lineOffsets_,0,1024*sizeof(unsigned char));
      // build the line corrections offsets
      for(line=0;line<height_;line++) {
      // read the black pixels
         temp=0;
         // first edge
         for(row=3;row<18;row++) {
            temp+=image_[1558*line+row];
         }
         // second edge
         for(row=1301;row<1309;row++) {
            temp+=image_[1558*line+row];
         }
         // average value
         lineOffsets_[line]=clip(temp/23);
      }
   }

   if(frameAvailable) {
      for(line=0;line<height_;line++) {
         for(row=0;row<width_;row++) {
            if(denoise) {
               // filtering
               image[offset]=clip((int)image_[1558*line+20+row+xpos_]-(int)lineOffsets_[line]);
            } else {
               // no filter
               image[offset]=image_[1558*line+20+row+xpos_];
            }
            offset++;
         }
      }
      frameAvailable=FALSE;
      pthread_mutex_unlock(&exposureMutex);
      return(1);
   }
   pthread_mutex_unlock(&exposureMutex);
   return(0);
}


// set the autoguide port corrections
// direction : QHY_NORTH to QHY_WEST
// duration in ms, 0 to cancel
// one move per call
int QHY5cam::move(int direction, int duration) {
   int res;
   int pulses[2]={-1,-1};

   // stopping
   if((duration==0)||(direction==QHY_NONE)||(direction==QHY_STOP_NS)||(direction==QHY_STOP_EW)) {
      switch(direction) {
         case QHY_NORTH :
         case QHY_SOUTH :
         case QHY_STOP_NS :
            direction=0x22;
            break;
         case QHY_EAST :
         case QHY_WEST :
         case QHY_STOP_EW :
            direction=0x21;
            break;
         case QHY_NONE :
         default :
            direction=0x18;
      }
      pthread_mutex_lock(&usbMutex);
      res=usb_control_msg(dev,0xc2,direction,0,0,NULL,0,500);
      pthread_mutex_unlock(&usbMutex);
      return(res);
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
   pthread_mutex_lock(&usbMutex);
   res=usb_control_msg(dev,0x42,0x10,0,direction,(char*)pulses,sizeof(pulses),500);
   pthread_mutex_unlock(&usbMutex);
   return(res);
}

// moves until stopped
int QHY5cam::move(int direction) {
   pthread_mutex_lock(&moveLoopMutex);
   switch(direction) {
      case QHY_NORTH :
         move_south_=FALSE;
         move_north_=TRUE;
         break;
      case QHY_SOUTH :
         move_north_=FALSE;
         move_south_=TRUE;
         break;
      case QHY_EAST :
         move_west_=FALSE;
         move_east_=TRUE;
         break;
      case QHY_WEST :
         move_east_=FALSE;
         move_west_=TRUE;
         break;
      case QHY_STOP_NS :
         move_north_=FALSE;
         move_south_=FALSE;
         break;
      case QHY_STOP_EW :
         move_east_=FALSE;
         move_west_=FALSE;
         break;
      case QHY_NONE :
      default :
         move_east_=FALSE;
         move_west_=FALSE;
         move_south_=FALSE;
         move_north_=FALSE;
   }
   pthread_mutex_unlock(&moveLoopMutex);
   return(move(direction,2000));
}

// configure the cam
int  QHY5cam::configure(int xpos, int ypos, int w, int h, int gg1, int bg, int rg, int gg2, int* rw=NULL, int* rh=NULL) {
   char registers[19];
   int offset,index,value,res;
   const int setgain[GAIN_SCALE_SIZE]={
                   0x000,0x004,0x005,0x006,0x007,0x008,0x009,0x00A,0x00B,
                   0x00C,0x00D,0x00E,0x00F,0x010,0x011,0x012,0x013,0x014,
                   0x015,0x016,0x017,0x018,0x019,0x01A,0x01B,0x01C,0x01D,
                   0x01E,0x01F,0x020,0x051,0x052,0x053,0x054,0x055,0x056,
                   0x057,0x058,0x059,0x05A,0x05B,0x05C,0x05D,0x05E,0x05F,
                   0x060,0x061,0x062,0x063,0x064,0x065,0x066,0x067,0x6CF,
                   0x6D0,0x6D1,0x6D2,0x6D3,0x6D4,0x6D5,0x6D6,0x6D7,0x6D8,
                   0x6D9,0x6DA,0x6DB,0x6DC,0x6DD,0x6DE,0x6DF,0x6E0,0x6E1,
                   0x6E2,0x6E3,0x6E4,0x6E5,0x6E6,0x6E7,0x6FC,0x6FD,0x6FE,
                   0x6FF};

   // setting registers
   width_=w;
   height_=h-(h%4);
   if(width_>1280) width_=1280;
   if(width_<1) width_=1;
   if(height_>1024) height_=1024;
   if(height_<1) height_=1;
   if(rw) *rw=width_;
   if(rh) *rh=height_;

   xpos_=xpos;
   ypos_=ypos;
   if(xpos_<0) xpos_=0;
   if(ypos_<1) ypos_=0;
   if((xpos_+width_)>1280) width_=1280-xpos_;
   if((ypos_+height_)>1024) height_=1024-ypos_;

   gg1_=gg1;
   bg_=bg;
   rg_=rg;
   gg2_=gg2;
   if(gg1_<0) gg1_=0;
   if(gg1_>=GAIN_SCALE_SIZE) gg1_=GAIN_SCALE_SIZE-1;
   if(bg_<0) bg_=0;
   if(bg_>=GAIN_SCALE_SIZE) bg_=GAIN_SCALE_SIZE-1;
   if(rg_<0) rg_=0;
   if(rg_>=GAIN_SCALE_SIZE) rg_=GAIN_SCALE_SIZE-1;
   if(gg2_<0) gg2_=0;
   if(gg2_>=GAIN_SCALE_SIZE) gg2_=GAIN_SCALE_SIZE-1;
   gg1_=setgain[gg1_];
   bg_=setgain[bg_];
   rg_=setgain[rg_];
   gg2_=setgain[gg2_];

   size_=1558*(height_+26);
   offset=(1048-height_)/2;
   index=(1558*(height_+26))>>16;
   value=(1558*(height_+26))&0xffff;
   STORE_WORD_BE(registers+0,gg1_);	// register 0x2B
   STORE_WORD_BE(registers+2,bg_);	// register 0x2C
   STORE_WORD_BE(registers+4,rg_);	// register 0x2D
   STORE_WORD_BE(registers+6,gg2_);	// register 0x2E
   STORE_WORD_BE(registers+8,offset);	// row start 0x01
   STORE_WORD_BE(registers+10,0);	// column start 0x02
   STORE_WORD_BE(registers+12,height_-1);	// row size 0x03
   STORE_WORD_BE(registers+14,0x0521);	// column size 0x04
   STORE_WORD_BE(registers+16,height_+25);	// register 0x09
   registers[18]=0xcc;

   pthread_mutex_lock(&usbMutex);
   res=usb_control_msg(dev,0x42,0x13,value,index,registers,19,500);
   usb_control_msg(dev,0x42,0x14,0x31a5,0,NULL,0,500);	// starting row offset ?
   usb_control_msg(dev,0x42,0x16,0,0,NULL,0,500);	// really used ??
   pthread_mutex_unlock(&usbMutex);

   // alloc mem
   free(image_);
   image_=(unsigned char*)malloc(size_);

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


void* QHY5cam::moveLoop() {
   bool state;

   pthread_mutex_lock(&moveLoopMutex);
   state=moveLoop_on_;
   pthread_mutex_unlock(&moveLoopMutex);

   while(state) {
      sleep(1);
      pthread_mutex_lock(&moveLoopMutex);
      if(move_north_) move(QHY_NORTH,2000);
      if(move_south_) move(QHY_SOUTH,2000);
      if(move_east_) move(QHY_EAST,2000);
      if(move_west_) move(QHY_WEST,2000);
      state=moveLoop_on_;
      pthread_mutex_unlock(&moveLoopMutex);
   }
   pthread_exit(NULL);
}


QHY5cam::QHY5cam() {
   int temp1,temp2;
   struct usb_bus* bus;
   struct usb_device* device;

   // init
   dev=NULL;
   image_=NULL;
   move_east_=FALSE;
   move_west_=FALSE;
   move_north_=FALSE;
   move_south_=FALSE;
   moveLoop_on_=TRUE;
   frameAvailable=FALSE;

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

   // init usb mutex
   pthread_mutex_init(&usbMutex,NULL);

   // init exposure mutex
   pthread_mutex_init(&exposureMutex,NULL);

   // init the move loop mutex
   pthread_mutex_init(&moveLoopMutex,NULL);

   // init config
   configure(0,0,1280,1024,1,1,1,1,&temp1,&temp2);

   // start the loop thread
   pthread_create(&moveLoopThread, NULL, QHY5cam::callMoveLoop, this);
}

QHY5cam::~QHY5cam() {
   // stop the loop thread, step 1
   pthread_mutex_lock(&moveLoopMutex);
   moveLoop_on_=FALSE;
   pthread_mutex_unlock(&moveLoopMutex);
   // release the interface
   usb_release_interface(dev,0);
   // close usb device
   usb_close(dev);
   // free buffer
   free(image_);
   // stop the loop thread, step 2
   pthread_join(moveLoopThread, NULL);
   pthread_mutex_destroy(&moveLoopMutex);
   pthread_mutex_destroy(&exposureMutex);
   pthread_mutex_destroy(&usbMutex);
}

// gives os time in second (usec accuracy)
unsigned long QHY5cam::getTime() {
   unsigned long t;
   struct timeval tv;
   gettimeofday(&tv,NULL);
   t=tv.tv_usec;
   t+=tv.tv_sec*1000000;
   return(t);
}
