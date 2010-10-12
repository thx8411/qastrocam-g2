/******************************************************************
Qastrocam
Copyright (C) 2003-2009   Franck Sicard
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


#ifndef _QTelescopeAutostar_hpp_
#define _QTelescopeAutostar_hpp_

#include "QTelescope.hpp"
#include <string>

using namespace std;

/** controle a Meade telescope via the serial port of
    the Autostar controler.
    At this time there is a bug: it is possible to send
    data to the autostar, but not to receive. */
class QTelescopeAutostar : public QTelescope {
   Q_OBJECT;
public:
   QTelescopeAutostar(const char * deviceName);
   ~QTelescopeAutostar();
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
protected:
   /** send a command to the Autostar, wait the result and
       return it. */
   enum CommandType {
      park,
      getAlignment,
      setAlignment,
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
   enum TrackingMode {
      land,
      polar,
      altAz,
      german
   };
   enum SubVersion {
      versionDate,
      versionFull,
      versionNumber,
      versionTime,
      productName
   };
   enum ReturnType {
      singleChar,
      booleans,
      numerics,
      strings,
      none
   };
   string sendCommand(CommandType com,const string & param="");
   void setTracking(TrackingMode);
   string version(SubVersion v);
private:
   /** really send the command. */
   bool sendCmd(const string & cmd,const string & param="");
   /** read the result. */
   string recvCmd(ReturnType t);
   // the file decritor of the serial port.
   int descriptor_;
   TrackingMode aligment_;
};

#endif
