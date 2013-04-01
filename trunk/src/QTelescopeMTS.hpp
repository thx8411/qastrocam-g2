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


#ifndef _QTelescopeMTS_hpp_
#define _QTelescopeMTS_hpp_

#include "QTelescope.hpp"
#include <string>

using namespace std;

/** Control PowerFlex MTS drive controllers from Boxdoerfer
    Elektronik via RS-232. Based on the manual of version 2.00.

    Supported models: MTS-3, MTS-3LP, MTS3-SLP, MTS-3SDI

    NOTES:
      - setTracking not implemented
      - Speed values 0.0-0.9 sets SLOW mode, 0.9-1.0 sets FAST mode
    */

class QTelescopeMTS : public QTelescope {
   Q_OBJECT
public:
   QTelescopeMTS(const char * deviceName);
   ~QTelescopeMTS();
   void buildGUI(QWidget * parent);
   virtual int telescopeType() { return(TELESCOPE_MTS); };
public slots:
   virtual void goE(float s=0) { sendCommand( righton ); };
   virtual void goW(float s=0) { sendCommand( lefton ); };
   virtual void goS(float s=0) { sendCommand( downon ); };
   virtual void goN(float s=0) { sendCommand( upon ); };
   virtual void stopE()        { sendCommand( rightoff ); };
   virtual void stopN()        { sendCommand( upoff ); };
   virtual void stopW()        { sendCommand( leftoff ); };
   virtual void stopS()        { sendCommand( downoff ); };
   virtual double setSpeed(double speed);
   virtual bool setTracking(bool activated);
protected:
  /* Commands without return value */
   enum CommandType {
      upon,
      upoff,
      downon,
      downoff,
      righton,
      rightoff,
      lefton,
      leftoff,
      faston,
      fastoff,
      onon,
      onoff,
      displayoff
   };
  /* Method to handle non-returning commands */
   void sendCommand( CommandType );

  /* Commands with 1 byte return value (RD1)*/
   enum RD1CommandType {
     ready,
     readswitch
   };
  /* Method to handle RD1 commands */
   int sendCommand( RD1CommandType );
private:
   /* Send the command to serial port */
   bool sendCmd( int cmd );
   /* Read the one Byte result from serial port */
   int recvRD1Cmd();
   /* The file descpritor of the serial port. */
   int descriptor_;
};

#endif
