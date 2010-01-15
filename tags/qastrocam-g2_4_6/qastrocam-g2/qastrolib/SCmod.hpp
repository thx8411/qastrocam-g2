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


#ifndef _SCmod_hpp_
#define _SCmod_hpp_

/** Abstract class to control an SC moded webcam
 */

#include <string>

using namespace std;

class QWidget;

class SCmod {
public:
   virtual void enterLongPoseMode() {};
   virtual void leaveLongPoseMode() {};
   virtual void stopAccumulation() {};
   virtual void startAccumulation() {};
   virtual ~SCmod();
   virtual QWidget * buildGUI(QWidget * parent) { return parent; }
   void setLevels(bool polarity);
protected :
   bool inverted_;
};


class QCamVesta;
class SCmodTucLed : public SCmod {
public:
   SCmodTucLed(QCamVesta & cam);
   void enterLongPoseMode();
   void leaveLongPoseMode();
   void stopAccumulation();
   void startAccumulation();
private:
   QCamVesta & cam_;
};

class SCmodSerialPort : public SCmod {
public:
   SCmodSerialPort();
   void enterLongPoseMode();
   void leaveLongPoseMode();
   void stopAccumulation();
   void startAccumulation();
private:
   string device;
   int device_;
};

#endif
