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

#include <qmessagebox.h>

#include "QTelescopeLX200.moc"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

using namespace std;

QTelescopeLX200::QTelescopeLX200(const char * deviceName) :
   QTelescope() {
   struct termios termios_p;
   currentSpeed=0.0;

   descriptor_=open(deviceName,O_RDWR|O_NOCTTY);
   if (descriptor_==-1) {
      perror(deviceName);
      QMessageBox::information(0,"Qastrocam-g2","Unable to reach the telescope device\nThe mount won't move...");
   }

   memset(&termios_p,0,sizeof(termios_p));
   termios_p.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
   termios_p.c_iflag = IGNPAR;
   termios_p.c_oflag = 0;   /* Raw output */

   /* set input mode (non-canonical, no echo, ... */
   termios_p.c_lflag = 0;

   termios_p.c_cc[VTIME] = 1; /* inter-character timer unused, block instead */
   termios_p.c_cc[VMIN] = 1;  /* read 1 character minimum */

   tcflush(descriptor_, TCIFLUSH);             /* clear the channel */
   tcsetattr(descriptor_,TCSANOW,&termios_p);

   setSpeed(0.1);
}

QTelescopeAutostar::~QTelescopeAutostar() {
   close(descriptor_);
}

void QTelescopeAutostar::buildGUI(QWidget * parent) {
   QTelescope::buildGUI(parent);
   widget()->setCaption(version(versionFull).c_str());
}

string QTelescopeAutostar::sendCommand(CommandType com,const string & param) {
   ssize_t tmp;
   switch (com) {
   case moveWest:
      sendCmd("Mw");
      break;
   case moveEast:
      sendCmd("Me");
      break;
   case moveNorth:
      sendCmd("Mn");
      break;
   case moveSouth:
      sendCmd("Ms");
      break;
   case stopMoveSouth:
      sendCmd("Qs");
      break;
   case stopMoveNorth:
      sendCmd("Qn");
      break;
   case stopMoveEast:
      sendCmd("Qe");
      break;
   case stopMoveWest:
      sendCmd("Qw");
      break;
   case park:
      sendCmd("hP");
      break;
   case setAlignment:
      sendCmd("A",param);
      break;
   case getAlignment:
      {
         char szACK[1]={(char)0x06};
         tmp=write(descriptor_,szACK,1);
         return recvCmd(singleChar);
      }
      break;
   case setMoveSpeed:
      sendCmd("Sw",param);
      break;
   }
   return "";
}

bool QTelescopeAutostar::sendCmd(const string & cmd,const string & param) {
   // '#' removed at the beginning of the command
   // not really needed for autostar, depsite the Meade docs
   // now also supports the LX200 telescopes
   string fullCmd=string(":")+cmd+param+"#";
   cout <<"sending command '"<<fullCmd<<"'"<<endl;
   if (fullCmd.length() != write(descriptor_,
                                 fullCmd.c_str(),
                                 fullCmd.length())) {
      perror(fullCmd.c_str());
      close(descriptor_);
      descriptor_=-1;
      return false;
   }
   return true;
}

double QTelescopeAutostar::setSpeed(double speed) {
   if (speed <=0.3) {
      speed=0.3;
      if(speed!=currentSpeed) {
         sendCommand(setMoveSpeed,"2");
         currentSpeed=speed;
      }
   } else if (speed <=0.6) {
      speed=0.6;
      if(speed!=currentSpeed) {
         sendCommand(setMoveSpeed,"3");
         currentSpeed=speed;
      }
   } else /*if (speed <=3/3)*/ {
      speed=1.0;
      if(speed!=currentSpeed) {
         sendCommand(setMoveSpeed,"4");
         currentSpeed=speed;
      }
   }
   return(currentSpeed);
}

bool QTelescopeAutostar::setTracking(bool activated) {
   if (aligment_ == land) {
      return !activated;
   }
   // set Aligment mode not working
   return activated;
}

void QTelescopeAutostar::setTracking(TrackingMode mode) {
   switch (mode) {
   case polar:
      sendCommand(setAlignment,"P");
      break;
   case german:
      sendCommand(setAlignment,"G");
      break;
   case altAz:
      sendCommand(setAlignment,"A");
      break;
   case land:
      sendCommand(setAlignment,"L");
      break;
   }
}

