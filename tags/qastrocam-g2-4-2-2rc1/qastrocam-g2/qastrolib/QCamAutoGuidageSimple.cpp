#include "QCamAutoGuidageSimple.moc"

#include "QTelescope.hpp"
#include "QCam.hpp"
#include "ShiftInfo.hpp"
#include <qhbox.h>
#include <qvbox.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include "QCamUtilities.hpp"

#include <iostream>

TrackingControl::TrackingControl(QString label, QWidget * parent, const char * name, WFlags f ) :
   QHBox(parent,name,f),
   label_(label,this),
   arrow_(this),
   currentShift_(4,this),
   labelMin_(tr("min"),this),
   minShift_(this),
   labelMax_(tr("max"),this),
   maxShift_(this) {
   currentShift_.setSmallDecimalPoint(true);
   setMin(2);
   setMax(10);
   connect(&minShift_,SIGNAL(textChanged(const QString&)),
           this,SLOT(setMin(const QString&)));
   connect(&maxShift_,SIGNAL(textChanged(const QString&)),
           this,SLOT(setMax(const QString&)));
   label_.setMinimumWidth(30);
}

void TrackingControl::setShift(double shift) {
   currentShift_.display(round(shift));
}
void TrackingControl::setMin(int min) {
   min_=min;
   minShift_.setText(QString().sprintf("%d",min_));
   emit(minChanged(min_));
}
void TrackingControl::setMax(int max) {
   max_=max;
   maxShift_.setText(QString().sprintf("%d",max_));
   emit(maxChanged(min_));
}

void TrackingControl::setMin(const QString & val) {
    setMin(atoi(val.ascii()));
}

void TrackingControl::setMax(const QString & val) {
   setMax(atoi(val.ascii()));
}

void TrackingControl::setMoveDir(MoveDir move) {
   switch(move) {
   case MovedNorth:
      arrow_.setPixmap(*QCamUtilities::getIcon("up.png"));
      break;
   case MovedSouth:
      arrow_.setPixmap(*QCamUtilities::getIcon("down.png"));
      break;
   case MovedEast:
      arrow_.setPixmap(*QCamUtilities::getIcon("left.png"));
      break;
   case MovedWest:
      arrow_.setPixmap(*QCamUtilities::getIcon("right.png"));
      break;
   case NotMoved:
      break;
   }
   arrow_.setDisabled(move == NotMoved);
}

QCamAutoGuidageSimple::QCamAutoGuidageSimple() {
   ewSwapped_=false;
   nsSwapped_=false;
   lastAltMove_=NotMoved;
   lastAscMove_=NotMoved;
   minShift_=0.2;
   maxShift_=50;
   centerMode_=false;
}

QWidget * QCamAutoGuidageSimple::buildGUI(QWidget *parent) {
   QWidget * mainBox = QCamAutoGuidage::buildGUI(parent);

   cout << "QCamAutoGuidageSimple::buildGUI()"<<endl;
   
   QHBox * buttons=new QHBox(mainBox);
   QCheckBox * swapEWb = new QCheckBox(tr("swap E/W"),buttons);
   connect(swapEWb,SIGNAL(toggled(bool)),this,SLOT(swapEW(bool)));
   QCheckBox* swapNSb = new QCheckBox(tr("swap N/S"),buttons);
   connect(swapNSb,SIGNAL(toggled(bool)),this,SLOT(swapNS(bool)));
   QCheckBox* centerb = new QCheckBox(tr("Center"),buttons);
   connect(centerb,SIGNAL(toggled(bool)),this,SLOT(setCenter(bool)));

   TrackingControl * trAlt = new TrackingControl(tr("Alt."),mainBox);
   connect(this,SIGNAL(shiftAlt(double)),trAlt,SLOT(setShift(double)));
   connect(trAlt,SIGNAL(minChanged(int)),this,SLOT(setMinShift(int)));
   connect(trAlt,SIGNAL(maxChanged(int)),this,SLOT(setMaxShift(int)));
   connect(this,SIGNAL(altMove(MoveDir)),trAlt,SLOT(setMoveDir(MoveDir)));

   TrackingControl * trAsc = new TrackingControl(tr("Asc."),mainBox);
   connect(this,SIGNAL(shiftAsc(double)),trAsc,SLOT(setShift(double)));
   connect(trAsc,SIGNAL(minChanged(int)),this,SLOT(setMinShift(int)));
   connect(trAsc,SIGNAL(maxChanged(int)),this,SLOT(setMaxShift(int)));
   connect(this,SIGNAL(ascMove(MoveDir)),trAsc,SLOT(setMoveDir(MoveDir)));
           
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
      if (vs.x()<-minShift_) {
         if (!ewSwapped_) asc=MovedEast; else asc=MovedWest;
      } else if (vs.x()>minShift_){
         if (!ewSwapped_) asc=MovedWest ; else asc=MovedEast;
      }
   } else {
      asc=NotMoved;
   }
   if (vs.y() != 0) {
      if (vs.y()>minShift_) {
         if (!nsSwapped_) alt=MovedSouth ; else  alt=MovedNorth;
      } else if (vs.y()<-minShift_) {
         if (!nsSwapped_)  alt=MovedNorth ; else  alt=MovedSouth;
      }
   } else {
      alt=NotMoved;
   }

   emit(shiftAsc(vs.x()));
   emit(shiftAlt(vs.y()));
   
   switch (asc) {
   case NotMoved:
      moveAsc(NotMoved);
      break;
   case MovedEast:
   case MovedWest:
      if (lastAscMove_== asc
          || lastAscMove_ == NotMoved
          || fabs(vs.x())>maxShift_) {
         moveAsc(asc);
      } else {
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
      if (lastAltMove_== alt
          || lastAltMove_ == NotMoved
          || fabs(vs.y())>maxShift_) {
         moveAlt(alt);
      } else {
         moveAlt(NotMoved);
      }
      break;
   case MovedEast:
   case MovedWest:
      cerr << "invalid shift to east/west received\n";
      break;
   }
}
