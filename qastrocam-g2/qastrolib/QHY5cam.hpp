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
#define QHY_NORTH	0x20
#define QHY_SOUTH	0x40
#define QHY_EAST	0x10
#define QHY_WEST	0x80

// features
#define QHY_IMAGER	0x00
#define QHY_GUIDER	0x01

class QHY5cam {
public :
   // singleton stuff

   // get a new instance from the singleton (imager or guider)
   static QHY5cam* instance(int feature);
   // distroy the instance
   static void destroy(int feature);

   // class functions

   // is the cam plugged ?
   static bool plugged();

private :
   // functions
   QHY5cam();
   ~QHY5cam();
   // vars
   static QHY5cam* instance_;
   static bool feature_used[2];
};

#endif
