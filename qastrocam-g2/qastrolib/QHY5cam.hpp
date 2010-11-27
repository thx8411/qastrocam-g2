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

// Interface for the QHY5 native driver
// singleton class

// Tom's firmware only supports timed moves ont the ST4 port.
// For permanent moves, this driver use a periodic thread, refreshing
// the timed moves until stop

#ifndef _QHY5CAM_HPP_
#define _QHY5CAM_HPP_

// threaded code must be re-entrant
#ifndef _REENTRANT
#define _REENTRANT
#endif
#include <pthread.h>

using namespace std;

// guider directions
#define QHY_NORTH	0
#define QHY_SOUTH	1
#define QHY_EAST	2
#define QHY_WEST	3
#define QHY_NONE	4
#define	QHY_STOP_NS	5
#define QHY_STOP_EW	6

// camera's features
#define QHY_IMAGER	0
#define QHY_GUIDER	1

class QHY5cam {
public :
   // singleton stuff

   // get a new instance from the singleton (imager or guider)
   static QHY5cam* instance(int feature);
   // distroy the instance
   static void destroy(int feature);

   // class functions
   // reset the cam
   int stop();
   // start a single shot picture
   int shoot(int duration);
   // read the last picture shot
   int read(char* image);
   // ST4 port timed move
   int move(int direction, int duration);
   // ST4 permanent move
   int move(int direction);
   // configure the camera
   int configure(int xpos, int ypos, int w, int h, int gg1, int b, int r, int gg2, int* rw, int* rh);

   // is the cam plugged ?
   static bool plugged();

private :
   // functions
   QHY5cam();
   ~QHY5cam();
   // timed moves loop thread
   static void* callMoveLoop(void* arg) { return(((QHY5cam*)arg)->moveLoop()); }
   void* moveLoop();

   // singleton vars
   static QHY5cam* instance_;
   static bool feature_used[2];

   // usb device
   struct usb_dev_handle* dev;

   // configuration
   char* image_;
   bool frameAvailable;
   int xpos_;
   int ypos_;
   int width_;
   int height_;
   int size_;

   // gains
   int gg1_, bg_, rg_, gg2_;

   // active moves
   bool move_east_, move_west_, move_north_, move_south_;

   // usb mutex
   pthread_mutex_t usbMutex;

   // timed moves loop thread
   pthread_t moveLoopThread;
   pthread_mutex_t moveLoopMutex;
   bool moveLoop_on_;
};

#endif
