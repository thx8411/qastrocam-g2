#include "QTelescopeMTS.moc"

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

   cout << "PowerFlex MTS telescope control\n";

   descriptor_ = open( deviceName, O_RDWR | O_NOCTTY );
   if ( descriptor_== -1 ) {
      perror(deviceName);
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
     cerr << "Error: MTS controller busy\n\n";
     exit(0);
   }
   /* Display redirection must be disabled during PC operation */
   sendCommand ( displayoff );

   cout << "\nSetting speed to 0.1\n";
   setSpeed( 0.1 );

}

void QTelescopeMTS::buildGUI(QWidget * parent) {
   QTelescope::buildGUI(parent);
   widget()->setCaption("PowerFlex MTS");
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
   if ( speed <= 0.9 ) {
     sendCommand( fastoff ); // SLOW
     return 0.1;
   } else {
     sendCommand( faston );  // FAST
      return 1.0;
   }
}

/* Sets the tracking of the mount on/off
   NOT IMPLEMENTED */
bool QTelescopeMTS::setTracking(bool activated) {
  cout << "setTracking not implemented.";
  return activated;
}
