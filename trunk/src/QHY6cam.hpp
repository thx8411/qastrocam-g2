/******************************************************************
Qastrocam-g2
Copyright (C) 2010-2014 Blaise-Florentin Collin

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

// Interface for the QHY6 native driver
// singleton class

// Tom's firmware only supports timed moves ont the ST4 port.
// For permanent moves, this driver use a periodic thread, refreshing
// the timed moves until stop

#ifndef _QHY6CAM_HPP_
#define _QHY6CAM_HPP_

// only if usb available
#if HAVE_USB_H && HAVE_PTHREADS_H

// threaded code must be re-entrant
#ifndef _REENTRANT
#define _REENTRANT
#endif
#include <pthread.h>

using namespace std;

// guider directions
#ifndef QHY_NORTH
#define QHY_NORTH	0
#define QHY_SOUTH	1
#define QHY_EAST	2
#define QHY_WEST	3
#define QHY_NONE	4
#define	QHY_STOP_NS	5
#define QHY_STOP_EW	6
#endif

// camera's features
#define QHY6_IMAGER	0
#define QHY6_GUIDER	1

class QHY6cam {
public :
   // singleton stuff

   // get a new instance from the singleton (imager or guider)
   static QHY6cam* instance(int feature);
   // distroy the instance
   static void destroy(int feature);

   // class functions
   // reset the cam
   int stop();
   // start a single shot picture
   int shoot(int duration, bool mode=FALSE);
   // read the last picture shot
   int read(char* image, bool mode=FALSE);
   // ST4 port timed move
   int move(int direction, int duration);
   // ST4 permanent move
   int move(int direction);
   // configure the camera
   int configure(int exposure, int gain, int offset=115, int amp=2, int speed=0);

   // is the cam plugged ?
   static bool plugged();

private :
   // functions
   QHY6cam();
   ~QHY6cam();
   // timed moves loop thread
   static void* callMoveLoop(void* arg) { return(((QHY6cam*)arg)->moveLoop()); }
   void* moveLoop();

   // debug tool
   // gives os time in second (usec accuracy)
   unsigned long getTime();

   // singleton vars
   static QHY6cam* instance_;
   static bool feature_used[2];

   // usb device
   struct usb_dev_handle* dev;

   // active moves
   bool move_east_, move_west_, move_north_, move_south_;

   // usb mutex
   pthread_mutex_t usbMutex;

   // timed moves loop thread
   pthread_t moveLoopThread;
   pthread_mutex_t moveLoopMutex;
   bool moveLoop_on_;
};

#endif /* HAVE_USB_H && HAVE_PTHREADS_H */

#endif
