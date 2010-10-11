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

#include "QTelescopeNexstar.moc"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>

using namespace std;

QTelescopeNexstar::QTelescopeNexstar(const char * deviceName) : QTelescope() {
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

void QTelescopeNexstar::buildGUI(QWidget * parent) {
   QTelescope::buildGUI(parent);
   widget()->setCaption("Nexstar");
}

void QTelescopeNexstar::goE(float shift) {
   //
}

void QTelescopeNexstar::goW(float shift) {
   //
}

void QTelescopeNexstar::goS(float shift) {
   //
}

void QTelescopeNexstar::goN(float shift) {
   //
}

void QTelescopeNexstar::stopE() {
   //
}

void QTelescopeNexstar::stopN() {
   //
}

void QTelescopeNexstar::stopW() {
   //
}

void QTelescopeNexstar::stopS() {
   //
}

double QTelescopeNexstar::setSpeed(double speed) {
   //
   return(currentSpeed);
}

bool QTelescopeNexstar::setTracking(bool activated) {
   //
   return activated;
}

