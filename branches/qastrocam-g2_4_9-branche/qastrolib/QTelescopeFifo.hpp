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


#ifndef _QTelescopeFifo_hpp_
#define _QTelescopeFifo_hpp_

#include "QTelescope.hpp"

/** Telescope implementation for a generic fifo interface.
 */
class QTelescopeFifo : public QTelescope {
   Q_OBJECT;
public:
   QTelescopeFifo(const char * fifoPath);
   ~QTelescopeFifo();
   virtual int telescopeType() { return(TELESCOPE_FIFO); }
public slots:
   virtual void goE(float shift) {}
   virtual void goW(float shift) {}
   virtual void goS(float shift) {}
   virtual void goN(float shift) {}
   virtual void stopW() {}
   virtual void stopE() {}
   virtual void stopN() {}
   virtual void stopS() {}
   virtual double setSpeed(double speed) {return(0); }
   virtual bool setTracking(bool activated) {return true; }
private:
   int descriptor_;
};

#endif
