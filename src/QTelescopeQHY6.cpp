/******************************************************************
Qastrocam-g2
Copyright (C) 2010-2014   Blaise-Florentin Collin

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

// only available if have usb
#if (HAVE_USB_H && HAVE_PTHREADS_H)

#include <Qt/qmessagebox.h>

#include "QTelescopeQHY6.hpp"

#include <stdlib.h>
#include <iostream>
#include <string.h>

#include "SettingsBackup.hpp"

using namespace std;

extern settingsBackup settings;

QTelescopeQHY6::QTelescopeQHY6() : QTelescope() {
   // is there a cam ?
   if(!QHY6cam::plugged()) {
      cam_=NULL;
      QMessageBox::information(0,"Qastrocam-g2","Unable to reach the QHY6 guider\nThe mount won't move...");
   } else {
      // get the cam instance
      cam_=QHY6cam::instance(QHY6_GUIDER);
      if(cam_==NULL) {
         QMessageBox::information(0,"Qastrocam-g2","Unable to reach the QHY6 guider\nThe mount won't move...");
      } else {
         stopE();
         stopW();
         stopN();
         stopS();
      }
   }
}

QTelescopeQHY6::~QTelescopeQHY6() {
   // release the guider
   if(cam_) {
      QHY6cam::destroy(QHY6_GUIDER);
   }
}

void QTelescopeQHY6::buildGUI(QWidget * parent) {
   QTelescope::buildGUI(parent);
   widget()->setWindowTitle("QHY5 Autoguide");
}

void QTelescopeQHY6::goE(float shift) {
   if(cam_) {
      stopW();
      cam_->move(QHY_EAST);
   }
}

void QTelescopeQHY6::goW(float shift) {
   if(cam_) {
      stopE();
      cam_->move(QHY_WEST);
   }
}

void QTelescopeQHY6::goS(float shift) {
   if(cam_) {
      stopN();
      cam_->move(QHY_SOUTH);
   }
}

void QTelescopeQHY6::goN(float shift) {
   if(cam_) {
      stopS();
      cam_->move(QHY_NORTH);
   }
}

void QTelescopeQHY6::stopE() {
   if(cam_) {
      cam_->move(QHY_STOP_EW);
   }
}

void QTelescopeQHY6::stopN() {
   if(cam_) {
      cam_->move(QHY_STOP_NS);
   }
}

void QTelescopeQHY6::stopW() {
   if(cam_) {
      cam_->move(QHY_STOP_EW);
   }
}

void QTelescopeQHY6::stopS() {
   if(cam_) {
      cam_->move(QHY_STOP_NS);
   }
}

double QTelescopeQHY6::setSpeed(double speed) {
   return(0);
}

bool QTelescopeQHY6::setTracking(bool activated) {
   if(cam_) {
      // always tracking
      return activated;
   } else {
      return(!activated);
   }
}

#endif /* HAVE_USB_H && HAVE_PTHREADS_H */
