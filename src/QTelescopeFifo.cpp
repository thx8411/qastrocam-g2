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

#include "QTelescopeFifo.moc"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>

using namespace std;

QTelescopeFifo::QTelescopeFifo(const char * deviceName) :
   QTelescope() {
   descriptor_=open(deviceName,O_RDWR|O_NOCTTY);
   if (descriptor_==-1) {
      // try to create the fifo
      mkfifo(deviceName,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
      descriptor_=open(deviceName,O_RDWR|O_NOCTTY);
      if(descriptor_==-1) {
         perror(deviceName);
         QMessageBox::information(0,"Qastrocam-g2","Unable to reach the telescope device\nThe mount won't move...");
      }
   }
}

QTelescopeFifo::~QTelescopeFifo() {
   close(descriptor_);
}
