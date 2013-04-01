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


#ifndef _QTelescopeAPM_hpp_
#define _QTelescopeAPM_hpp_

#include "QTelescope.hpp"

#include "PPort.hpp"

/** Telescope implementation for APM interface. **/
class QTelescopeAPM : public QTelescope {
   Q_OBJECT
public:
   QTelescopeAPM(const char * pport);
   ~QTelescopeAPM();
   virtual int telescopeType() { return(TELESCOPE_APM); }
public slots:
   virtual void goE(float shift);
   virtual void goW(float shift);
   virtual void goS(float shift);
   virtual void goN(float shift);
   virtual void stopW();
   virtual void stopE();
   virtual void stopN();
   virtual void stopS();
   virtual double setSpeed(double speed);
   virtual bool setTracking(bool activated);
private:
   // bit values for stop and start
   bool go;
   bool stop;
   // port entry in PPdev table
   int portEntry;
   // port device name
   const char* portName;
   // PPdev object
   PPort* paralPort;
   // used bits
   enum BitControl { EastBit=4,WestBit=5,NorthBit=6,SouthBit=7};
};

#endif
