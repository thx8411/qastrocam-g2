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

#include <Qt/qlabel.h>
#include <Qt/qpushbutton.h>
#include <Qt/qmessagebox.h>

#include <Qt3Support/q3hbox.h>

#include "QCamVBox.hpp"
#include "QCamAutoGuidage.hpp"
#include "QCam.hpp"
#include "QTelescope.hpp"
#include "QCamFindShift.hpp"
#include "QCamUtilities.hpp"
#include "SettingsBackup.hpp"


extern settingsBackup settings;


#if HAVE_SDL_H && USE_SDL_AUDIO
//
// SDL audio callback
//

void SDL_AudioCallback(void* userdata, Uint8* stream, int len) {
   if(userdata) {
      memset(stream, 0, len);
      QCamAutoGuidage* parent=(QCamAutoGuidage*)userdata;
      if(len < (parent->wavLength - parent->wavPosition)) {
         memcpy(stream, parent->wavBuffer + parent->wavPosition, len);
         parent->wavPosition += len;
      } else
         parent->wavPosition = 0;
   }
}
#endif /* HAVE_SDL_H */

//
// class implementation
//

QCamAutoGuidage::QCamAutoGuidage() {
   cam_=NULL;
   tracker_=NULL;
   telescope_=NULL;
#if HAVE_SDL_H && USE_SDL_AUDIO
   wavPosition=0;
#else
   bell_=NULL;
#endif /* HAVE_SDL_H */
   alert_=NULL;
   isTracking_=false;
   isGuiding_=false;
   alertAscOn_=false;
   alertAltOn_=false;
   soundAlertOn_=false;
   bellOn_=false;

#if HAVE_SDL_H && USE_SDL_AUDIO
   static char variable[64];
   sprintf(variable,"SDL_AUDIODRIVER=alsa");
   putenv(variable);

   if(SDL_WasInit(SDL_INIT_AUDIO)==0)
      SDL_InitSubSystem(SDL_INIT_AUDIO);

   reqSpec.freq=22050;
   reqSpec.format=AUDIO_S16;
   reqSpec.channels=1;
   reqSpec.samples=4096;
   reqSpec.callback=SDL_AudioCallback;
   reqSpec.userdata=(void*)this;

   SDL_OpenAudio(&reqSpec,&devSpec);

   if(SDL_LoadWAV("/usr/share/qastrocam-g2/sounds/bell.wav", &wavSpec, &wavBuffer, &wavLength)==NULL) {
      cerr << "Unable to load the bell.wav sound." << endl;
   }
#else
   // test for audio device
   if(!QSound::isAvailable()) {
      cout << "Unable to use the audio device" << endl;
      if(!settings.haveKey("GUI_NAS_MESSAGE")||(settings.haveKey("GUI_NAS_MESSAGE")&&(strcasecmp(settings.getKey("GUI_NAS_MESSAGE"),"yes")==0))) {
         QMessageBox::information(0,"Qastrocam-g2","Unable to reach the audio device\nNo sound alerts for guiding.\nIs NAS installed ?");
         settings.setKey("GUI_NAS_MESSAGE","no");
      }
   } else {
      // setting the alert bell
      bell_=new QSound("/usr/share/qastrocam-g2/sounds/bell.wav");
      bell_->setLoops(-1);
   }
#endif /* HAVE_SDL_H */
}

QCamAutoGuidage::~QCamAutoGuidage() {
#if HAVE_SDL_H && USE_SDL_AUDIO
   SDL_PauseAudio(1);
   SDL_CloseAudio();
   SDL_FreeWAV(wavBuffer);
   SDL_QuitSubSystem(SDL_INIT_AUDIO);
#else
   // release the bell (really needed ?)
   if(bell_)
      delete bell_;
#endif
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
   QCamVBox * mainBox = new QCamVBox(parent);

   QCamUtilities::registerWidget(mainBox);

   if (parent == 0) {
      QCamUtilities::setQastrocamIcon(mainBox);
   }
   Q3HBox * buttons=new Q3HBox(mainBox);
   QPushButton * trackButton = new QPushButton(tr("track"),buttons);
   trackButton->setToggleButton(true);
   connect(trackButton,SIGNAL(toggled(bool)),this,SLOT(track(bool)));

   //
   track(false);
   //

   QPushButton * resetButton = new QPushButton(tr("reset"),buttons);
   connect(resetButton,SIGNAL(pressed()),tracker_,SLOT(reset()));

   // visual alert
   Q3HBox* state=new Q3HBox(mainBox);
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
      if(soundAlertOn_) {
#if HAVE_SDL_H && USE_SDL_AUDIO
         SDL_PauseAudio(0);
#else
         if(bell_)
            bell_->play();
#endif /* HAVE_SDL_H */
         }
         bellOn_=true;
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
      if(soundAlertOn_) {
#if HAVE_SDL_H && USE_SDL_AUDIO
         SDL_PauseAudio(1);
#else
         if(bell_)
            bell_->stop();
#endif /* HAVE_SDL_H */
      }
      bellOn_=false;
   }
}

void QCamAutoGuidage::soundAlertChanged(bool s) {
   if(s) {
      soundAlertOn_=true;
      if(bellOn_) {
#if HAVE_SDL_H && USE_SDL_AUDIO
         SDL_PauseAudio(0);
#else
         if(bell_)
            bell_->play();
#endif /* HAVE_SDL_H */
      }
      settings.setKey("GUIDE_ALERT","yes");
   } else {
      soundAlertOn_=false;
      if(bellOn_) {
#if HAVE_SDL_H && USE_SDL_AUDIO
         SDL_PauseAudio(1);
#else
         if(bell_)
            bell_->stop();
#endif /* HAVE_SDL_H */
      }
      settings.setKey("GUIDE_ALERT","no");
   }
}
