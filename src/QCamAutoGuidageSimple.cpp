/******************************************************************
Qastrocam
Copyright (C) 2003-2009   Franck Sicard
Qastrocam-g2
Copyright (C) 2009-2013 Blaise-Florentin Collin

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

#include <iostream>

#include <Qt/qpixmap.h>
#include <Qt/qpushbutton.h>
#include <Qt/qcheckbox.h>

#include "QCamAutoGuidageSimple.hpp"
#include "QTelescope.hpp"
#include "QCam.hpp"
#include "ShiftInfo.hpp"
#include "QCamUtilities.hpp"
#include "SettingsBackup.hpp"

// settings object
extern settingsBackup settings;

//
// TrackingControl
//

TrackingControl::TrackingControl(QString label, QWidget * parent, const char * name, Qt::WFlags f ) :
QCamHBox(parent,name,f),
label_(label,this),
arrow_(this),
currentShift_(4,this),
labelMin_(tr("min"),this),
minShift_(this),
labelMax_(tr("max"),this),
maxShift_(this)
{
   currentShift_.setSmallDecimalPoint(true);

   // read the previous min value
   /*keyMin_="GUIDE_";
   keyMin_+=label;
   keyMin_+="_MIN";
   if(settings.haveKey(keyMin_.ascii())) {
      setMin(QString(settings.getKey(keyMin_.ascii())));
   } else
      setMin(2);*/

   // read the previous max value
   /*keyMax_="GUIDE_";
   keyMax_+=label;
   keyMax_+="_MAX";
   if(settings.haveKey(keyMax_.ascii())) {
      setMax(QString(settings.getKey(keyMax_.ascii())));
   } else
      setMax(10);*/

   connect(&minShift_,SIGNAL(textChanged(const QString&)),this,SLOT(setMin(const QString&)));
   connect(&maxShift_,SIGNAL(textChanged(const QString&)),this,SLOT(setMax(const QString&)));

   QPixmap* tmpIcon;
   tmpIcon=QCamUtilities::getPixmap("movie_pause.png");
   if(tmpIcon!=NULL) {
      arrow_.setPixmap(*tmpIcon);
      delete(tmpIcon);
   } else
      arrow_.setText("Pause");

   label_.setMinimumWidth(30);
}

void TrackingControl::init() {
   // read the previous min value
   keyMin_="GUIDE_";
   keyMin_+=label_.text();
   keyMin_+="_MIN";
   if(settings.haveKey(keyMin_.ascii())) {
      setMin(QString(settings.getKey(keyMin_.ascii())));
   } else
      setMin(2);

   // read the previous max value
   keyMax_="GUIDE_";
   keyMax_+=label_.text();
   keyMax_+="_MAX";
   if(settings.haveKey(keyMax_.ascii())) {
      setMax(QString(settings.getKey(keyMax_.ascii())));
   } else
      setMax(10);
}

void TrackingControl::setShift(double shift) {
   currentShift_.display(round(shift));
}
void TrackingControl::setMin(int min) {
   min_=min;
   minShift_.setText(QString().sprintf("%d",min_));
   emit(minChanged((double)min_));

   // save the value
   settings.setKey(keyMin_.ascii(),minShift_.text().ascii());
}
void TrackingControl::setMax(int max) {
   max_=max;
   maxShift_.setText(QString().sprintf("%d",max_));
   emit(maxChanged((double)max_));

   // save the value
   settings.setKey(keyMax_.ascii(),maxShift_.text().ascii());
}

void TrackingControl::setMin(const QString & val) {
    setMin(atoi(val.ascii()));
}

void TrackingControl::setMax(const QString & val) {
   setMax(atoi(val.ascii()));
}

void TrackingControl::setMoveDir(MoveDir move) {
   QPixmap* tmpIcon;
   switch(move) {
   case MovedNorth:
      tmpIcon=QCamUtilities::getPixmap("up.png");
      if(tmpIcon!=NULL) {
         arrow_.setPixmap(*tmpIcon);
         delete(tmpIcon);
      } else
         arrow_.setText("North");
      break;
   case MovedSouth:
      tmpIcon=QCamUtilities::getPixmap("down.png");
      if(tmpIcon!=NULL) {
         arrow_.setPixmap(*tmpIcon);
         delete(tmpIcon);
      } else
         arrow_.setText("South");
      break;
   case MovedEast:
      tmpIcon=QCamUtilities::getPixmap("left.png");
      if(tmpIcon!=NULL) {
         arrow_.setPixmap(*tmpIcon);
         delete(tmpIcon);
      } else
         arrow_.setText("West");
      break;
   case MovedWest:
      tmpIcon=QCamUtilities::getPixmap("right.png");
      if(tmpIcon!=NULL) {
         arrow_.setPixmap(*tmpIcon);
         delete(tmpIcon);
      } else
         arrow_.setText("East");
      break;
   case NotMoved:
      tmpIcon=QCamUtilities::getPixmap("movie_pause.png");
      if(tmpIcon!=NULL) {
         arrow_.setPixmap(*tmpIcon);
         delete(tmpIcon);
      } else
         arrow_.setText("Pause");
      break;
   }
}

//
// QCamAutoGuidageSimple
//

QCamAutoGuidageSimple::QCamAutoGuidageSimple() {
   ewSwapped_=false;
   nsSwapped_=false;
   lastAltMove_=NotMoved;
   lastAscMove_=NotMoved;
   minAscShift_=0.2;
   maxAscShift_=50;
   minAltShift_=0.2;
   maxAltShift_=50;
   centerMode_=false;
}

void QCamAutoGuidageSimple::swapEW(bool swap) {
   ewSwapped_=swap;
   // update settings
   if(swap) {
      settings.setKey("GUIDE_RA_INVERSION","yes");
   } else {
      settings.setKey("GUIDE_RA_INVERSION","no");
   }
}

void QCamAutoGuidageSimple::swapNS(bool swap) {
   nsSwapped_=swap;
   // update settings
   if(swap) {
      settings.setKey("GUIDE_DEC_INVERSION","yes");
   } else {
      settings.setKey("GUIDE_DEC_INVERSION","no");
   }
}

void QCamAutoGuidageSimple::setCenter(bool center) {
   centerMode_=center;
   // update settings
   if(center) {
      settings.setKey("GUIDE_CENTER","yes");
   } else {
      settings.setKey("GUIDE_CENTER","no");
   }
}

QWidget * QCamAutoGuidageSimple::buildGUI(QWidget *parent) {
   QWidget * mainBox = QCamAutoGuidage::buildGUI(parent);

   QCamHBox * buttons=new QCamHBox(mainBox);

   // RA inversion checkbutton
   QCheckBox * swapEWb = new QCheckBox(tr("swap E/W"),buttons);
   if(settings.haveKey("GUIDE_RA_INVERSION")) {
      if(QString(settings.getKey("GUIDE_RA_INVERSION"))=="yes") {
         swapEWb->setChecked(true);
         swapEW(true);
      }
   }
   connect(swapEWb,SIGNAL(toggled(bool)),this,SLOT(swapEW(bool)));

   // DEC inversion checkbutton
   QCheckBox* swapNSb = new QCheckBox(tr("swap N/S"),buttons);
   if(settings.haveKey("GUIDE_DEC_INVERSION")) {
      if(QString(settings.getKey("GUIDE_DEC_INVERSION"))=="yes") {
         swapNSb->setChecked(true);
         swapNS(true);
      }
   }
   connect(swapNSb,SIGNAL(toggled(bool)),this,SLOT(swapNS(bool)));

   // center mode checkbox
   QCheckBox* centerb = new QCheckBox(tr("Center"),buttons);
   if(settings.haveKey("GUIDE_CENTER")) {
      if(QString(settings.getKey("GUIDE_CENTER"))=="yes") {
         centerb->setChecked(true);
         setCenter(true);
      }
   }
   connect(centerb,SIGNAL(toggled(bool)),this,SLOT(setCenter(bool)));

   TrackingControl * trAlt = new TrackingControl(tr("DEC"),mainBox);
   connect(this,SIGNAL(shiftAlt(double)),trAlt,SLOT(setShift(double)));
   connect(trAlt,SIGNAL(minChanged(double)),this,SLOT(setAltMinShift(double)));
   connect(trAlt,SIGNAL(maxChanged(double)),this,SLOT(setAltMaxShift(double)));
   connect(this,SIGNAL(altMove(MoveDir)),trAlt,SLOT(setMoveDir(MoveDir)));
   trAlt->init();

   TrackingControl * trAsc = new TrackingControl(tr("RA"),mainBox);
   connect(this,SIGNAL(shiftAsc(double)),trAsc,SLOT(setShift(double)));
   connect(trAsc,SIGNAL(minChanged(double)),this,SLOT(setAscMinShift(double)));
   connect(trAsc,SIGNAL(maxChanged(double)),this,SLOT(setAscMaxShift(double)));
   connect(this,SIGNAL(ascMove(MoveDir)),trAsc,SLOT(setMoveDir(MoveDir)));
   trAsc->init();

   swapEWb->show();
   swapNSb->show();
   centerb->show();
   buttons->show();
   trAlt->show();
   trAsc->show();
   mainBox->show();
   return mainBox;
}

void QCamAutoGuidageSimple::frameShift(const ShiftInfo & shift) {
   MoveDir asc=NotMoved;
   MoveDir alt=NotMoved;

   Vector2D vs=shift.shift();
   if (centerMode_) {
      Vector2D oc(cam()->size().width()/2,
                  cam()->size().height()/2);
      vs=shift.center()-oc;
   }

   if (vs.x() != 0) {
      if (vs.x()<-minAscShift_) {
         if (!ewSwapped_) asc=MovedEast; else asc=MovedWest;
      } else if (vs.x()>minAscShift_){
         if (!ewSwapped_) asc=MovedWest ; else asc=MovedEast;
      } else
         stopAlert(GUIDE_ASC);
   } else {
      asc=NotMoved;
   }
   if (vs.y() != 0) {
      if (vs.y()>minAltShift_) {
         if (!nsSwapped_) alt=MovedSouth ; else  alt=MovedNorth;
      } else if (vs.y()<-minAltShift_) {
         if (!nsSwapped_)  alt=MovedNorth ; else  alt=MovedSouth;
      } else
         stopAlert(GUIDE_ALT);
   } else {
      alt=NotMoved;
   }

   emit(shiftAsc(vs.x()));
   emit(shiftAlt(vs.y()));
   telescope_->Update(vs.x(),vs.y());

   switch (asc) {
   case NotMoved:
      moveAsc(NotMoved);
      break;
   case MovedEast:
   case MovedWest:
      if ((lastAscMove_== asc|| lastAscMove_ == NotMoved)&& fabs(vs.x())<maxAscShift_) {
            stopAlert(GUIDE_ASC);
         moveAsc(asc);
      } else {
         if(fabs(vs.x())>maxAscShift_)
            startAlert(GUIDE_ASC);
         moveAsc(NotMoved);
      }
      break;
   case MovedNorth:
   case MovedSouth:
      cerr << "invalid shift to north/south received\n";
      break;
   }
   switch (alt) {
   case NotMoved:
      moveAlt(NotMoved);
      break;
   case MovedNorth:
   case MovedSouth:
      if ((lastAltMove_== alt|| lastAltMove_ == NotMoved)&& fabs(vs.y())<maxAltShift_) {
            stopAlert(GUIDE_ALT);
         moveAlt(alt);
      } else {
         if((fabs(vs.y())>maxAltShift_))
            startAlert(GUIDE_ALT);
         moveAlt(NotMoved);
      }
      break;
   case MovedEast:
   case MovedWest:
      cerr << "invalid shift to east/west received\n";
      break;
   }
}
