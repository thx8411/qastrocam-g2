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


#ifndef _QTelescopeLX200_hpp_
#define _QTelescopeLX200_hpp_

#include "QTelescope.hpp"
#include <string>

using namespace std;

/** controle a Meade telescope via the LX200 serial port **/
class QTelescopeLX200 : public QTelescope {
   Q_OBJECT
public:
   QTelescopeLX200(const char * deviceName);
   ~QTelescopeLX200();
   virtual int telescopeType() { return(TELESCOPE_LX200); }
   void buildGUI(QWidget * parent);
private:
   enum CommandType {
      moveSouth,
      moveNorth,
      moveEast,
      moveWest,
      stopMoveEast,
      stopMoveWest,
      stopMoveNorth,
      stopMoveSouth,
   };
   void sendCommand(CommandType c);
   void sendSpeed(string speed);
   // the file descritor of the serial port.
   int descriptor_;
public slots:
   virtual void goE(float shift) {sendCommand(moveEast);};
   virtual void goW(float shift) {sendCommand(moveWest);};
   virtual void goS(float shift) {sendCommand(moveSouth);};
   virtual void goN(float shift) {sendCommand(moveNorth);};
   virtual void stopE() {sendCommand(stopMoveEast);};
   virtual void stopN() {sendCommand(stopMoveNorth);};
   virtual void stopW() {sendCommand(stopMoveWest);};
   virtual void stopS() {sendCommand(stopMoveSouth);};
   virtual double setSpeed(double speed);
   virtual bool setTracking(bool activated);
};

#endif
