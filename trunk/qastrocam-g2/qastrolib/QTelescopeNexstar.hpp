/******************************************************************
Qastrocam-g2
Copyright (C) 2010   Blaise-Florentin Collin

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


#ifndef _QTelescopeNexstar_hpp_
#define _QTelescopeNexstar_hpp_

#include "QTelescope.hpp"
#include <string>

#define NEXSTAR_RA	16
#define NEXSTAR_DEC	17
#define NEXSTAR_UP	36
#define NEXSTAR_DOWN	37

using namespace std;

/** controle a Nexstar telescope via the serial port of
    the handbox controler.
**/
class QTelescopeNexstar : public QTelescope {
   Q_OBJECT;
public:
   QTelescopeNexstar(const char * deviceName);
   ~QTelescopeNexstar();
   void buildGUI(QWidget * parent);
public slots:
   virtual void goE(float shift);
   virtual void goW(float shift);
   virtual void goS(float shift);
   virtual void goN(float shift);
   virtual void stopE();
   virtual void stopN();
   virtual void stopW();
   virtual void stopS();
   virtual double setSpeed(double speed);
   virtual bool setTracking(bool activated);
protected:
private:
   char message[8];
   void move(int direction, int sens, int rate);
   // the file descriptor of the serial port.
   int descriptor_;
};

#endif
