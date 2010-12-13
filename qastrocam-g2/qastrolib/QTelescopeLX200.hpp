/******************************************************************
Qastrocam-g2
Copyright (C) 2009-2010   Blaise-Florentin Collin

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

/** controle a Meade telescope via the serial port of
    the Autostar controler.
    At this time there is a bug: it is possible to send
    data to the autostar, but not to receive. */
class QTelescopeLX200 : public QTelescope {
   Q_OBJECT;
public:
   QTelescopeLX200(const char * deviceName);
   ~QTelescopeLX200();
   virtual int telescopeType() { return(TELESCOPE_LX200); }
   void buildGUI(QWidget * parent);
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
private:
   /** send a command to the Autostar, wait the result and
       return it. */
   enum CommandType {
      moveSouth,
      moveNorth,
      moveEast,
      moveWest,
      stopMoveEast,
      stopMoveWest,
      stopMoveNorth,
      stopMoveSouth,
      setMoveSpeed
   };
   /** really send the command. */
   void sendCommand(CommandType c);
   // the file descritor of the serial port.
   int descriptor_;
};

#endif
