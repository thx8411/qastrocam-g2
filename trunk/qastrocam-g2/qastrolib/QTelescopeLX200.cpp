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

QTelescopeLX200::~QTelescopeLX200() {
   close(descriptor_);
}

void QTelescopeLX200::buildGUI(QWidget* parent) {
   QTelescope::buildGUI(parent);
   widget()->setCaption("LX200 generic");
}

void QTelescopeLX200::sendCommand(CommandType c) {
   string msg;
   switch (c) {
   case moveWest:
      msg=":Mw#";
      break;
   case moveEast:
      msg=":Me#";
      break;
   case moveNorth:
      msg=":Mn#";
      break;
   case moveSouth:
      msg=":Ms#";
      break;
   case stopMoveSouth:
      msg=":Qs#";
      break;
   case stopMoveNorth:
      msg=":Qn#";
      break;
   case stopMoveEast:
      msg=":Qe#";
      break;
   case stopMoveWest:
      msg=":Qw#";
      break;
   }
   if (msg.length() != write(descriptor_,msg.c_str(),msg.length())) {
      perror(msg.c_str());
      close(descriptor_);
      descriptor_=-1;
   }
}

void QTelescopeLX200::sendSpeed(string speed){
   string msg;
   msg+=":Sw";
   msg+=speed;
   msg+="#";
   if (msg.length() != write(descriptor_,msg.c_str(),msg.length())) {
      perror(msg.c_str());
      close(descriptor_);
      descriptor_=-1;
   }
}

double QTelescopeLX200::setSpeed(double speed) {
   if (speed <=0.3) {
      speed=0.3;
      if(speed!=currentSpeed) {
         sendSpeed("2");
         currentSpeed=speed;
      }
   } else if (speed <=0.6) {
      speed=0.6;
      if(speed!=currentSpeed) {
         sendSpeed("3");
         currentSpeed=speed;
      }
   } else /*if (speed <=3/3)*/ {
      speed=1.0;
      if(speed!=currentSpeed) {
         sendSpeed("4");
         currentSpeed=speed;
      }
   }
   return(currentSpeed);
}

bool QTelescopeLX200::setTracking(bool activated) {
   // always tracking
   return activated;
}


