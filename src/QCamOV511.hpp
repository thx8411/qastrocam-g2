/******************************************************************
Qastrocam
Copyright (C) 2003-2009   Franck Sicard
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


#ifndef _QCamOV511_hpp_
#define _QCamOV511_hpp_

#include "QCamV4L2.hpp"
/** enhance QCamV4L2 to handle specifities of OV511 webcams. */

class QCamOV511 : public QCamV4L2 {
   Q_OBJECT
public:
   QCamOV511(const char * devpath="/dev/video0");
};

class QCamOV519 : public QCamV4L2 {
   Q_OBJECT
public:
   QCamOV519(const char * devpath="/dev/video0");
};

#endif
