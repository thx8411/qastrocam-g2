/******************************************************************
Qastrocam-g2
Copyright (C) 2010 Blaise-Florentin Collin

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

// QHY6 native driver access
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

#include "QHY6cam.hpp"

//
// singleton stuff
//

// static members init
QHY6cam* QHY6cam::instance_=NULL;
bool QHY6cam::feature_used[2]={false,false};

// get the class instance for a feature. Create the singleton if needed
QHY6cam* QHY6cam::instance(int feature) {
   // claim device part
   if(feature>=2)
      return(NULL);
   if(instance_==NULL) {
      instance_=new QHY6cam();
      feature_used[feature]=true;
      return(instance_);
   }
   if(feature_used[feature])
      return(NULL);
   return(instance_);
}

// release the device part
void QHY6cam::destroy(int feature) {
   feature_used[feature]=false;
   if(!(feature_used[QHY6_IMAGER] || feature_used[QHY6_GUIDER])) {
      delete instance_;
      instance_=NULL;
   }
}

//
// functions
//

// class functions

// resets the cam
int QHY6cam::stop() {
   int res;

   // is this cam able to stop ?

   return(res);
}

// start picture shoot (duration in ms)
// mode : true -> short exposure
int QHY6cam::shoot(int duration, bool mode) {
   int ret;

   //

   return(ret);
}

// read the picture
// image buffer size : x*y*1 byte
int QHY6cam::read(char* image, bool mode) {

   //

   return(0);
}


// set the autoguide port corrections
// direction : QHY_NORTH to QHY_WEST
// duration in ms, 0 to cancel
// one move per call
int QHY6cam::move(int direction, int duration) {
   unsigned int ret;
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
      res=usb_control_msg(dev,0xc2,direction,0,0,(char*)&ret,sizeof(&ret),500);
      pthread_mutex_unlock(&usbMutex);

      //
      cerr << "QHY6cam::move(int,int) : st4 move halted, usb_control_msg() returned : " << res << endl;
      //

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

   //
   cerr << "QHY6cam::move(int,int) : st4 moving, usb_control_msg() returned : " << res << endl;
   //
   return(res);
}

// moves until stopped
int QHY6cam::move(int direction) {
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
// exposure in ms
// gain : 0-63
int  QHY6cam::configure(int exposure, int gain, int offset, int amp, int speed) {
   unsigned char registers[24];

   exposure-=exposure/100;
   if(exposure<0) exposure=0;

   if(gain<0) gain=0;
   if(gain>63) gain=63;

   memset(registers,0,24 );

   registers[0]=gain;
   //registers[1]=;
   //registers[2]=;
   //registers[3]=;
   //

   registers[8]=0xAC;
   registers[9]=0xAC;

   //

   registers[23]=0xAC;

   //

   return(0);
}

// is the cam plugged ?
bool QHY6cam::plugged() {
   struct usb_bus* bus;
   struct usb_device* device;

   // update usb datas
   usb_init();
   usb_find_busses();
   usb_find_devices();
   // look for the device
   for(bus = usb_busses; bus; bus = bus->next) {
      for(device = bus->devices; device; device=device->next) {
         if((device->descriptor.idVendor == 0x16c0)&&(device->descriptor.idProduct == 0x081D)) {
            //
            cerr << "QHY6cam::plugged() : is the cam plugged ? -> yes" << endl;
            //
            return(true);
         }
      }
   }
   //
   cerr << "QHY6cam::plugged() : is the cam plugged ? -> no" << endl;
   //
   return(false);
}


void* QHY6cam::moveLoop() {
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


QHY6cam::QHY6cam() {
   int temp1,temp2;
   struct usb_bus* bus;
   struct usb_device* device;

   // init
   dev=NULL;
   move_east_=FALSE;
   move_west_=FALSE;
   move_north_=FALSE;
   move_south_=FALSE;
   moveLoop_on_=TRUE;

   // update usb datas
   usb_init();
   usb_find_busses();
   usb_find_devices();

   // get the usb device handle
   for(bus = usb_busses; bus; bus = bus->next) {
      for(device = bus->devices; device; device=device->next) {
         if((device->descriptor.idVendor == 0x16c0)&&(device->descriptor.idProduct == 0x081D)) {
            dev=usb_open(device);
            if(dev==NULL) {
               QMessageBox::information(0,"Qastrocam-g2","Unable to reach the QHY6 imager\nLeaving...");
               //
               cerr << "QHY6cam::QHY6cam() : usb_open() failed" << endl;
               //
               exit(1);
            }
            int res;
            res=usb_set_configuration(dev,1);

            //
            cerr << "QHY6cam::QHY6cam() : usb_set_configuration() returned :" << res << endl;
            //

            res=usb_claim_interface(dev,0);

            //
            cerr << "QHY6cam::QHY6cam() : usb_claim_interface() returned :" << res << endl;
            //

            res=usb_set_altinterface(dev,0);

            //
            cerr << "QHY6cam::QHY6cam() : usb_set_altinterface() returned :" << res << endl;
            //
         }
      }
   }

   // init usb mutex
   pthread_mutex_init(&usbMutex,NULL);

   // init the move loop mutex
   pthread_mutex_init(&moveLoopMutex,NULL);

   //

   // start the loop thread
   pthread_create(&moveLoopThread, NULL, QHY6cam::callMoveLoop, this);
}

QHY6cam::~QHY6cam() {
   // stop the loop thread, step 1
   pthread_mutex_lock(&moveLoopMutex);
   moveLoop_on_=FALSE;
   pthread_mutex_unlock(&moveLoopMutex);
   // release the interface
   usb_release_interface(dev,0);
   // close usb device
   usb_close(dev);
   // stop the loop thread, step 2
   pthread_join(moveLoopThread, NULL);
   pthread_mutex_destroy(&moveLoopMutex);
   pthread_mutex_destroy(&usbMutex);
}

// gives os time in second (usec accuracy)
unsigned long QHY6cam::getTime() {
   unsigned long t;
   struct timeval tv;
   gettimeofday(&tv,NULL);
   t=tv.tv_usec;
   t+=tv.tv_sec*1000000;
   return(t);
}
