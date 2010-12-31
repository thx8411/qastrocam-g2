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


#include "QCamAutoGuidage.moc"

#include "QCam.hpp"
#include "QTelescope.hpp"
#include "QCamFindShift.hpp"
#include "QCamUtilities.hpp"
#include "SettingsBackup.hpp"

#include <qpushbutton.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qmessagebox.h>

extern settingsBackup settings;

QCamAutoGuidage::QCamAutoGuidage() {
   cam_=NULL;
   tracker_=NULL;
   telescope_=NULL;
   bell_=NULL;
   alert_=NULL;
   isTracking_=false;
   isGuiding_=false;
   alertAscOn_=false;
   alertAltOn_=false;
   soundAlertOn_=false;
   bellOn_=false;

   // test for audio device
   if(!(QSound::available()||QSound::isAvailable())) {
      cout << "Unable to use the audio device" << endl;
      QMessageBox::information(0,"Qastrocam-g2","Unable to reach the audio device\nNo sound alerts for guiding.\nIs NAS installed ?");
   } else {
      // setting the alert bell
      bell_=new QSound("/usr/share/qastrocam-g2/sounds/bell.wav");
      bell_->setLoops(-1);
   }
}

QCamAutoGuidage::~QCamAutoGuidage() {
   // release the bell (really needed ?)
   if(bell_)
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
      if(alert_) {
         alert_->setText("Not connected");
         alert_->setPaletteBackgroundColor(Qt::red);
      }
      return;
   }
   if (mode) {
      tracker_->connectCam(*cam_);
      connect(tracker_,SIGNAL(shift(const ShiftInfo&)),this,SLOT(frameShift(const ShiftInfo&)));
      telescope_->setTrack(true);
      isGuiding_=true;
      if(alert_) {
         alert_->setText("Guiding...");
         alert_->setPaletteBackgroundColor(Qt::green);
      }
   } else {
      tracker_->disconnectCam();
      disconnect(tracker_,SIGNAL(shift(const ShiftInfo&)),this,SLOT(frameShift(const ShiftInfo&)));
      telescope_->stopE();
      telescope_->stopW();
      telescope_->stopS();
      telescope_->stopN();
      telescope_->setTrack(false);
      isGuiding_=false;
      if(alert_) {
         alert_->setText("Idle");
         alert_->setPaletteBackgroundColor(Qt::lightGray);
      }
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

   //
   track(false);
   //

   QPushButton * resetButton = new QPushButton(tr("reset"),buttons);
   connect(resetButton,SIGNAL(pressed()),tracker_,SLOT(reset()));

   // visual alert
   QHBox* state=new QHBox(mainBox);
   //QLabel* label1=new QLabel("State : ",state);
   alert_=new QLabel("Idle",state);
   alert_->setPaletteBackgroundColor(Qt::lightGray);
   alert_->setAlignment(Qt::AlignHCenter);

   // sound alert checkbox
   soundAlert_=new QCheckBox("Enable the sound alert",mainBox);
   connect(soundAlert_,SIGNAL(toggled(bool)),this,SLOT(soundAlertChanged(bool)));
   if(settings.haveKey("GUIDE_ALERT")) {
      if(QString(settings.getKey("GUIDE_ALERT"))=="yes") {
         soundAlert_->setChecked(true);
         soundAlertChanged(true);
      } else {
         soundAlert_->setChecked(false);
         soundAlertChanged(false);
      }
   }

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

void QCamAutoGuidage::startAlert(int d) {
   if(!(alertAscOn_||alertAltOn_)) {
      // visual alert
      if(alert_) {
         alert_->setText("Star Lost !");
         alert_->setPaletteBackgroundColor(Qt::red);
      }
      // sound alert
      if(bell_) {
         if(soundAlertOn_) {
            cerr << "bell" << endl;
            bell_->play();
         }
         bellOn_=true;
      }
   }

   if(d==GUIDE_ASC)
      alertAscOn_=true;
   else
      alertAltOn_=true;
}

void QCamAutoGuidage::stopAlert(int d) {
   if(d==GUIDE_ASC)
      alertAscOn_=false;
   else
      alertAltOn_=false;

   if(!(alertAscOn_||alertAltOn_)) {
      // visual alert
      if(alert_) {
         if(isGuiding_) {
            alert_->setText("Guiding...");
            alert_->setPaletteBackgroundColor(Qt::green);
         } else {
            alert_->setText("Idle");
            alert_->setPaletteBackgroundColor(Qt::gray);
         }
      }

      // sound alert
      if(bell_) {
         if(soundAlertOn_) {
            cerr << "bell stop" << endl;
            bell_->stop();
         }
         bellOn_=false;
      }
   }
}

void QCamAutoGuidage::soundAlertChanged(bool s) {
   if(s) {
      soundAlertOn_=true;
      if(bell_&&bellOn_) {
         cerr << "bell" << endl;
         bell_->play();
      }
      settings.setKey("GUIDE_ALERT","yes");
   } else {
      soundAlertOn_=false;
      if(bell_&&bellOn_) {
         cerr << "bell stop" << endl;
         bell_->stop();
      }
      settings.setKey("GUIDE_ALERT","no");
   }
}
