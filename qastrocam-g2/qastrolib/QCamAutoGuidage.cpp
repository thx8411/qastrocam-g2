/******************************************************************
Qastrocam
Copyright (C) 2003-2009   Franck Sicard
Qastrocam-g2
Copyright (C) 2009   Blaise-Florentin Collin

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


#include "QCamAutoGuidage.moc"

#include "QCam.hpp"
#include "QTelescope.hpp"
#include "QCamFindShift.hpp"
#include "QCamUtilities.hpp"

#include <qpushbutton.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qmessagebox.h>

QCamAutoGuidage::QCamAutoGuidage() {
   cam_=NULL;
   tracker_=NULL;
   telescope_=NULL;
   bell_=NULL;
   alert_=NULL;
   isTracking_=false;
   lastState_="Idle";
   lastColor_=QColor(Qt::green);

   // test for audio device
   if(!(QSound::available()||QSound::isAvailable())) {
      cout << "Unable to use the audio device" << endl;
      QMessageBox::information(0,"Qastrocam-g2","Unable to reach the audio device\nNo sound alerts for guiding...");
   } else {
      // setting the alert bell
      bell_=new QSound("/usr/share/qastrocam-g2/sounds/bell.wav");
      bell_->setLoops(-1);
   }
}

QCamAutoGuidage::~QCamAutoGuidage() {
   // release the bell
   delete bell_;
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
      if(alert_)
         alert_->setText("Guiding...");
   } else {
      tracker_->disconnectCam();
      disconnect(tracker_,SIGNAL(shift(const ShiftInfo&)),
                 this,SLOT(frameShift(const ShiftInfo&)));
      telescope_->stopE();
      telescope_->stopW();
      telescope_->stopS();
      telescope_->stopN();
      telescope_->setTrack(false);
      if(alert_)
         alert_->setText("Idle");
   }
}

QWidget * QCamAutoGuidage::buildGUI(QWidget *parent) {
   QVBox * mainBox = new QVBox(parent);

   QCamUtilities::registerWidget(mainBox);

   if (parent == 0) {
      QCamUtilities::setQastrocamIcon(mainBox);
   }
   QHBox * buttons=new QHBox(mainBox);
   QPushButton * trackButton = new QPushButton(tr("track"),buttons);
   trackButton->setToggleButton(true);
   connect(trackButton,SIGNAL(toggled(bool)),this,SLOT(track(bool)));
   QPushButton * resetButton = new QPushButton(tr("reset"),buttons);
   connect(resetButton,SIGNAL(pressed()),tracker_,SLOT(reset()));

   // visual alert
   QHBox* state=new QHBox(mainBox);
   //QLabel* label1=new QLabel("State : ",state);
   alert_=new QLabel("Idle",state);
   alert_->setPaletteBackgroundColor(Qt::green);
   alert_->setAlignment(Qt::AlignHCenter);
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

void QCamAutoGuidage::startAlert() {
   // visual alert
   if(alert_) {
      lastState_=alert_->text();
      lastColor_=alert_->paletteBackgroundColor();
      alert_->setText("Star Lost !");
      alert_->setPaletteBackgroundColor(Qt::red);
   }

   // sound alert
   if(bell_)
      bell_->play();
}

void QCamAutoGuidage::stopAlert() {
   // visual alert
   if(alert_) {
      alert_->setText(lastState_);
      alert_->setPaletteBackgroundColor(lastColor_);
   }

   // sound alert
   if(bell_)
      bell_->stop();
}
