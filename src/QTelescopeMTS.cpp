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

#include <Qt/qmessagebox.h>

#include "QTelescopeMTS.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>

using namespace std;

QTelescopeMTS::QTelescopeMTS(const char * deviceName) :
   QTelescope() {
   struct termios termios_p;
   currentSpeed=0;

   cout << "PowerFlex MTS telescope control\n";

   descriptor_ = open( deviceName, O_RDWR | O_NOCTTY );
   if ( descriptor_== -1 ) {
      perror(deviceName);
      QMessageBox::information(0,"Qastrocam-g2","Unable to reach the telescope device\nThe mount won't move...");
   }

   memset( &termios_p, 0, sizeof(termios_p) );
   termios_p.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
   termios_p.c_iflag = IGNPAR;
   termios_p.c_oflag = 0;   /* Raw output */
   termios_p.c_lflag = 0;
   termios_p.c_cc[VTIME] = 0;
   termios_p.c_cc[VMIN] = 1;  /* read 1 character minimum */
   tcflush( descriptor_, TCIFLUSH );    /* clear the channel */
   tcsetattr( descriptor_, TCSANOW, &termios_p );

   /* Check if the controller is idle */
   if ( sendCommand( ready ) != 0 ) {
     QMessageBox::information(0,"Qastrocam-g2","Error: MTS controller busy");
     cout << "Error: MTS controller busy\n\n";
     //exit(0);
   }
   /* Display redirection must be disabled during PC operation */
   sendCommand ( displayoff );

   cout << "\nSetting speed to 0.1\n";
   setSpeed( 0.1 );

}

QTelescopeMTS::~QTelescopeMTS() {
   close(descriptor_);
}

void QTelescopeMTS::buildGUI(QWidget * parent) {
   QTelescope::buildGUI(parent);
   widget()->setWindowTitle("PowerFlex MTS");
}

/** Send command to telescope controller
   Some of the commands seen here not yet used by
   any of the methods */
void QTelescopeMTS::sendCommand( CommandType c ) {
   switch (c) {
   case upon:
     sendCmd(0xa4);
      break;
   case upoff:
     sendCmd(0xa5);
      break;
   case downon:
     sendCmd(0xa6);
      break;
   case downoff:
     sendCmd(0xa7);
      break;
   case lefton:
     sendCmd(0xa8);
      break;
   case leftoff:
     sendCmd(0xa9);
      break;
   case righton:
     sendCmd(0xaa);
      break;
   case rightoff:
     sendCmd(0xab);
      break;
   case faston:
     sendCmd(0xb0);
     break;
   case fastoff:
     sendCmd(0xb1);
     break;
   case onon:
     sendCmd(0xae);
     break;
   case onoff:
     sendCmd(0xaf);
     break;
   case displayoff:
     sendCmd(0xa1);
     break;
   }
}
int QTelescopeMTS::sendCommand( RD1CommandType c ) {
  switch( c ) {
   case ready:
     sendCmd(0xa2);
     return ( recvRD1Cmd() );
     break;
  case readswitch:
    sendCmd(0xb3);
    return ( recvRD1Cmd() );
   }
  return( -1 );
}

/* Send the command data through serial port */
bool QTelescopeMTS::sendCmd( int cmd ) {
  /*cout <<"sending command '"<<cmd<<"'"<<endl;*/

  if ( write( descriptor_, &cmd , 1 ) < 0 )
    { return false; }
  return true;
}

/* Receive one byte of data from serial port */
int QTelescopeMTS::recvRD1Cmd() {
  char buf[1];
  int lu = 0;
  buf[0] = 0xff;

  if ( ( lu = read( descriptor_, buf, 1 ) ) != 1 ) {
    cerr << "Expecting a single byte when reading from MTS, got "<<lu<<"\n";
    return ( -1 );
  }
  return buf[0];
}

/* Sets the slew speed. Powerflex MTS has two selectable
   speed, SLOW for guiding and FAST for targeting */
double QTelescopeMTS::setSpeed(double speed) {
   if ( speed <= 0.5 ) {
     speed=0.5;
     if(speed!=currentSpeed) {
        sendCommand( fastoff ); // SLOW
        currentSpeed=speed;
     }
   } else {
     speed=1.0;
     if(speed!=currentSpeed) {
        sendCommand( faston );  // FAST
        currentSpeed=speed;
     }
   }
   return(currentSpeed);
}

/* Sets the tracking of the mount on/off
   NOT IMPLEMENTED */
bool QTelescopeMTS::setTracking(bool activated) {
  cout << "setTracking not implemented.";
  return activated;
}
