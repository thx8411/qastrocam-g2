#include "QCamAutoGuidage.moc"

#include "QCam.hpp"
#include "QTelescope.hpp"
#include "QCamFindShift.hpp"
#include "QCamUtilities.hpp"

#include <qpushbutton.h>
#include <qhbox.h>
#include "qvbox.h"

QCamAutoGuidage::QCamAutoGuidage() {
   cam_=NULL;
   tracker_=NULL;
   telescope_=NULL;
   isTracking_=false;
}

void QCamAutoGuidage::setCam(QCam * cam) {
   cam_=cam;
}

void QCamAutoGuidage::setScope(QTelescope * scope) {
   telescope_=scope;
}

void QCamAutoGuidage::setTracker(QCamFindShift * tracker) {
   tracker_=tracker;
}

void QCamAutoGuidage::track(bool mode) {
   if (!(cam_ && tracker_ && telescope_)) {
      return;
   }
   if (mode) {
      tracker_->connectCam(*cam_);
      connect(tracker_,SIGNAL(shift(const ShiftInfo&)),
              this,SLOT(frameShift(const ShiftInfo&)));
      telescope_->setTrack(true);
   } else {
      tracker_->disconnectCam();
      disconnect(tracker_,SIGNAL(shift(const ShiftInfo&)),
                 this,SLOT(frameShift(const ShiftInfo&)));
      telescope_->stopE();
      telescope_->stopW();
      telescope_->stopS();
      telescope_->stopN();
      telescope_->setTrack(false);
   }
}

QWidget * QCamAutoGuidage::buildGUI(QWidget *parent) {
   QVBox * mainBox = new QVBox(parent);
   if (parent == 0) {
      QCamUtilities::setQastrocamIcon(mainBox);
   }
   QHBox * buttons=new QHBox(mainBox);
   QPushButton * trackButton = new QPushButton(tr("track"),buttons);
   trackButton->setToggleButton(true);
   connect(trackButton,SIGNAL(toggled(bool)),this,SLOT(track(bool)));
   QPushButton * resetButton = new QPushButton(tr("reset"),buttons);
   connect(resetButton,SIGNAL(pressed()),tracker_,SLOT(reset()));
   mainBox->show();
   return mainBox;
}

void QCamAutoGuidage::moveAsc(MoveDir EWmove) {
   if (EWmove == lastAscMove_) return;
   emit(ascMove(EWmove));
   lastAscMove_=EWmove;
   switch (EWmove) {
   case MovedEast:
      telescope_->goE(tracker_->currentShift().shift().x());
      break;
   case MovedWest:
      telescope_->goW(tracker_->currentShift().shift().x());
      break;
   case NotMoved:
      telescope_->stopW();
      telescope_->stopE();
      break;
   default:
      assert(0);
   }
}

void QCamAutoGuidage::moveAlt(MoveDir NSmove) {
   if (NSmove == lastAltMove_) return;
   emit(altMove(NSmove));
   lastAltMove_=NSmove;
   switch (NSmove) {
   case MovedNorth:
      telescope_->goN(tracker_->currentShift().shift().y());
      break;
   case MovedSouth:
      telescope_->goS(tracker_->currentShift().shift().y());
      break;
   case NotMoved:
      telescope_->stopN();
      telescope_->stopS();
      break;
   default:
      assert(0);
   }
}
