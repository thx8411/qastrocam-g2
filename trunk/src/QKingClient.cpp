/******************************************************************
Qastrocam
Copyright (C) 2003-2009   Franck Sicard
Qastrocam-g2
Copyright (C) 2009-2013   Blaise-Florentin Collin

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

#include <math.h>
#include <time.h>

#include <Qt/qpushbutton.h>
#include <Qt/qstatusbar.h>

#include "QKingClient.hpp"
#include "QCam.hpp"
#include "QCamUtilities.hpp"


QKingClient::QKingClient() {
   timeFirstFrame_=0;
   statusBar_=NULL;
}

bool QKingClient::registerFirstFrame() {
   if (QCamFindShift_hotSpot::registerFirstFrame()) {
      timeFirstFrame_=time(NULL);
      return true;
   } else {
      return false;
   }
}

bool QKingClient::findShift(ShiftInfo & shift) {
   if (QCamFindShift_hotSpot::findShift(shift)) {

      // calculate misalignment
      /*
        X= x1 + dx/2 - dy/(C.dt)
        Y= y1 + dx/(C.dt)+dy/2
      */

      Vector2D rightPos;
      int dt=(time(NULL)-timeFirstFrame_);

      static const double C=0.000072722;
      double dx=round(shift.shift().x()*10.0)/10.0;
      double dy=round(shift.shift().y()*10.0)/10.0;

      if (dt != 0) {
         rightPos+=Vector2D((dx/2.0)-dy/(C*dt),
                           dx/(C*dt)+(dy/2.0));
      }
      if (statusBar_) {
         QString stat;
         stat.sprintf("Orig:%d,%d time=%d shift=%1.1f,%1.1f move=%1.1f,%1.1f",
                      (int)firstHotSpot_.x(),(int)firstHotSpot_.y(),
                      dt,dx,dy,rightPos.x(),rightPos.y());
         statusBar_->showMessage(stat);
      }
      cam().annotate(firstHotSpot_+rightPos);
      return true;
   } else {
      return false;
   }
}

QWidget* QKingClient::buildGUI(QWidget* parent) {
   QWidget* w=QCamFindShift_hotSpot::buildGUI(parent);

   QCamUtilities::registerWidget(w);

   QPushButton* resetCenter = new QPushButton("reset",w);
   connect(resetCenter,SIGNAL(pressed()),this,SLOT(kingReset()));

   statusBar_=new QStatusBar(w);

   kingReset();

   return w;
}

void QKingClient::kingReset() {
   reset();
   if (statusBar_) {
         QString stat;
         stat.sprintf("Orig:%d,%d time=%d shift=%1.1f,%1.1f move=%1.1f,%1.1f",
                      (int)firstHotSpot_.x(),(int)firstHotSpot_.y(),
                      0,0.0,0.0,0.0,0.0);
         statusBar_->showMessage(stat);
   }
}
