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
   descriptor_=open(deviceName,O_RDWR|O_NOCTTY);
   if (descriptor_==-1) {
      perror(deviceName);
   }
#if 0
   if (tcgetattr(descriptor_,&termios_p)!=0) {
      perror("tcgetattr");
   }
   cfsetospeed(&termios_p,B9600);
   cfsetispeed(&termios_p,B9600);
   if (tcsetattr(descriptor_,TCSANOW,&termios_p)!=0) {
      perror("tcsetattr");
   }
   
   if (0!=fcntl(descriptor_,F_SETFL,O_NONBLOCK)) {
      perror("O_NONBLOCK");
   }
#else
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
#endif
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

void QTelescopeAutostar::buildGUI(QWidget * parent) {
   QTelescope::buildGUI(parent);
   widget()->setCaption(version(versionFull).c_str());
}

string QTelescopeAutostar::sendCommand(CommandType com,
                                       const string & param) {
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
         write(descriptor_,szACK,1);
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
   string fullCmd=string("#:")+cmd+param+"#";
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
         cerr << "expecting a 1 char when reading from autostar\n";
      }
      buf[1]=0;
      break;
   case numerics:
      lu=read(descriptor_,buf,4);
      if (lu!=4) {
         cerr <<"expecting a 4 digit number when reading from autostar\n"; 
      }
      buf[lu]=0;
   case strings:
      do {
         int cur=read(descriptor_,buf+lu,1);
         if (cur!=1) {
            cerr << "expecting a 1 char when reading a string from autostar\n";
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
      sendCommand(setMoveSpeed,"2");
      return 1.0/3;
   } else if (speed <=2.0/3) {
      sendCommand(setMoveSpeed,"3");
      return 2.0/3;
   } else /*if (speed <=3/3)*/ {
      sendCommand(setMoveSpeed,"4");
      return 3.0/3;
   }
}

bool QTelescopeAutostar::setTracking(bool activated) {
   if (aligment_ == land) {
      return !activated;
   }
#if 1
   // set Aligment mode not working
   return activated;
#else
   if (activated) {
      setTracking(aligment_);
   } else {
      setTracking(land);
   }
   return true;
#endif
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
