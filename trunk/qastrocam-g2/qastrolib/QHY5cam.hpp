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

#ifndef _QHY5CAM_HPP_
#define _QHY5CAM_HPP_

using namespace std;

// guider directions
#define QHY_NORTH	0
#define QHY_SOUTH	1
#define QHY_EAST	2
#define QHY_WEST	3
#define QHY_NONE	4
#define	QHY_STOP_NS	5
#define QHY_STOP_EW	6

// features
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
   int stop();
   int shoot(int duration);
   int read(char* image);
   int move(int direction, int duration);
   int move(int direction);
   int configure(int xpos, int ypos, int w, int h, int gain, int* rw, int* rh);

   // is the cam plugged ?
   static bool plugged();

private :
   // functions
   QHY5cam();
   ~QHY5cam();

   // vars
   static QHY5cam* instance_;
   static bool feature_used[2];
   // usb device
   struct usb_dev_handle* dev;
   // configuration
   char* image_;
   int xpos_;
   int ypos_;
   int width_;
   int height_;
   int gain_;
   int size_;
};

#endif
