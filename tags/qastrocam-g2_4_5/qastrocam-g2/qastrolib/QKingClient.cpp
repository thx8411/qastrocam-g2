#include <qpushbutton.h>
#include <qstatusbar.h>
#include <math.h>
#include "QKingClient.moc"
#include "QCam.hpp"

#include <math.h>
#include <time.h>

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
                      dt,
                      dx,
                      dy,
                      rightPos.x(),rightPos.y());
         statusBar_->message(stat);
      }
      cam().annotate(firstHotSpot_+rightPos);
      return true;
   } else {
      return false;
   }
}

QWidget * QKingClient::buildGUI(QWidget * parent) {
   QWidget *w=QCamFindShift_hotSpot::buildGUI(parent);
   QPushButton * resetCenter = new QPushButton("reset",w);
   connect(resetCenter,SIGNAL(pressed()),this,SLOT(reset()));

   statusBar_=new QStatusBar(w);
   return w;
}
