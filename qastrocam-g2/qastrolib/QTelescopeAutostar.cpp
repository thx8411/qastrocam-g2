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

#include <qmessagebox.h>

#include "QTelescopeAutostar.moc"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>

using namespace std;

QTelescopeAutostar::QTelescopeAutostar(const char * deviceName) :
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

   cout << "Version number: "<<flush<<version(versionFull)<<"\n";
   cout << "Getting aligment: "<<flush;
   string aligment=sendCommand(getAlignment);
   if (aligment=="P") {
      aligment_=polar;
      cout <<"Telescope in Polar mount\n";
   } else if (aligment=="A") {
      aligment_=altAz;
      cout <<"Telescope in Alt-Az mount\n";
   } else if (aligment=="L") {
      aligment_=land;
      cout <<"Telescope in Landscape mode\n";
   } else if (aligment=="G") {
      aligment_=german;
      cout <<"Telescope in German polar mount\n";
   }
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
      return false;
   }
   return true;
}

string QTelescopeAutostar::recvCmd(ReturnType t) {
   char buf[100];
   int lu=0;
   buf[0]=0;
   switch (t) {
   case singleChar:
   case booleans:
      lu=read(descriptor_,buf,1);
      if (lu!=1) {
         cout << "expecting a 1 char when reading from autostar\n";
      }
      buf[1]=0;
      break;
   case numerics:
      lu=read(descriptor_,buf,4);
      if (lu!=4) {
         cout <<"expecting a 4 digit number when reading from autostar\n";
      }
      buf[lu]=0;
   case strings:
      do {
         int cur=read(descriptor_,buf+lu,1);
         if (cur!=1) {
            cout << "expecting a 1 char when reading a string from autostar\n";
         }
         lu+=cur;
      } while (buf[lu-1]!='#');
      buf[lu-1]=0;
      break;
   case none:
      break;
   }
   return buf;
}

double QTelescopeAutostar::setSpeed(double speed) {
   if (speed <=1.0/3) {
      speed=1.0/3;
      if(speed!=currentSpeed) {
         sendCommand(setMoveSpeed,"2");
         currentSpeed=speed;
      }
   } else if (speed <=2.0/3) {
      speed=2.0/3;
      if(speed!=currentSpeed) {
         sendCommand(setMoveSpeed,"3");
         currentSpeed=speed;
      }
   } else /*if (speed <=3/3)*/ {
      speed=3.0/3;
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

string QTelescopeAutostar::version(SubVersion v) {
   string param;
   switch (v) {
   case versionDate:
      param="D";
      break;
   case versionFull:
      param="F";
      break;
   case versionNumber:
      param="N";
      break;
   case versionTime:
      param="T";
      break;
   case productName:
      param="P";
      break;
   }
   sendCmd("GV",param);
   return recvCmd(strings);
}
