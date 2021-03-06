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

#include "QTelescopeMCU.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>

using namespace std;

QTelescopeMCU::QTelescopeMCU(const char * deviceName) :
   QTelescope() {
   struct termios termios_p;
   currentSpeed=0;

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

   cout << "MCU update kit revision number: "<<flush<<version(versionFull)<<"\n";
   cout << "Getting aligment: "<<flush;
   string aligment=sendCommand(getAlignment);
   if (aligment=="P") {
      aligment_=polar;
      cout <<"Telescope in Polar mount mode\n";
   } else if (aligment=="A") {
      aligment_=altAz;
      cout <<"Telescope in Alt-Az mount mode\n";
   } else if (aligment=="L") {
      aligment_=land;
      cout <<"Telescope in Landscape mode\n";
   } else if (aligment=="G") {
      aligment_=german;
      cout <<"Telescope in German polar mount mode\n";
   }

   cout << "Setting mount to guide speed:\n";
   setSpeed('G');
}

QTelescopeMCU::~QTelescopeMCU() {
   close(descriptor_);
}

void QTelescopeMCU::buildGUI(QWidget * parent) {
   QTelescope::buildGUI(parent);
   widget()->setWindowTitle(version(versionFull).c_str());
}

string QTelescopeMCU::sendCommand(CommandType com,const string & param) {
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
   case getAlignment:
      {
         char szACK[1]={(char)0x06};
         tmp=write(descriptor_,szACK,1);
         return recvCmd(singleChar);
      }
      break;
   case setMoveSpeed:
      sendCmd("R",param);
      break;
   case park:
      cout << "park command not implemented";
      break;
   case setAlignment:
      cout << "setAlignment command not implemented";
      break;
   }
   return "";
}

bool QTelescopeMCU::sendCmd(const string & cmd,const string & param) {
   string fullCmd=string("#:")+cmd+param+"#";
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

string QTelescopeMCU::recvCmd(ReturnType t) {
// currently, the communication with the MCU update kit is very limited
// the only information passed back is the revision information and the
// fixed alignment "polar" (which is wrong I think, by rights the kit should
// have to pass back "german")
   char buf[100];
   int lu=0;
   buf[0]=0;
   if(descriptor_!=-1) {
      switch (t) {
         case singleChar:
         case booleans:
            lu=read(descriptor_,buf,1);
            if (lu!=1) {
               cerr << "expecting a single char when reading from MCU\n";
            }
         buf[1]=0;
         break;
         case numerics:
            lu=read(descriptor_,buf,4);
            if (lu!=4) {
               cerr <<"expecting a 4 digit number when reading from MCU\n";
            }
            buf[lu]=0;
         case strings:
            do {
               int cur=read(descriptor_,buf+lu,1);
               if (cur!=1) {
                  cerr << "expecting single chars when reading a string from MCU\n";
               }
               lu+=cur;
            } while (buf[lu-1]!='#');
            buf[lu-1]=0;
            break;
         case revision:
            do {
               int cur=read(descriptor_,buf+lu,10);
               lu+=cur;
            } while ( lu < 10);
            buf[lu]=0;
            break;
         case none:
            break;
      }
   }
   return buf;
}

void QTelescopeMCU::setSpeed(char speed) {
    switch(speed) {
	case 'G':
	case 'C':
	case 'M':
	case 'S':
	    char buf[2];

	    buf[0]=speed;
	    buf[1]=0;

	    sendCommand(setMoveSpeed,buf);
	    break;
	default:
	    cerr << "illegal move speed "<<speed<<"\n";
	    break;
    }
}

double QTelescopeMCU::setSpeed(double speed) {
   if (speed <=1.0/3) {
      speed=1.0/3;
      if(speed!=currentSpeed) {
         sendCommand(setMoveSpeed,"G");
         currentSpeed=speed;
      }
   } else if (speed <=2.0/3) {
      speed=2.0/3;
      if(speed!=currentSpeed) {
         sendCommand(setMoveSpeed,"C");
         currentSpeed=speed;
      }
   } else /*if (speed <=3/3)*/ {
      speed=3.0/3;
      if(speed!=currentSpeed) {
         sendCommand(setMoveSpeed,"M");
         currentSpeed=speed;
      }
   }
   return(currentSpeed);
}

bool QTelescopeMCU::setTracking(bool activated) {
   if (aligment_ == land) {
      return !activated;
   }
   return activated;
}

void QTelescopeMCU::setTracking(TrackingMode mode) {
// MCU update kit does not allow setting the tracking mode
// hence this function is void
}

string QTelescopeMCU::version(SubVersion v) {
// The MCU update kit only understands a single version command, therefore
// parameter v is ignored here
   sendCmd("G","V");
   return recvCmd(revision);
}
