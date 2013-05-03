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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

#include <Qt/qimage.h>
#include <Qt/qpushbutton.h>
#include <Qt/qtooltip.h>
#include <Qt/qcheckbox.h>

#include "QCamUtilities.hpp"
#include "QCamAutoAlign.hpp"
#include "QCamFindShift.hpp"
#include "QHistogram.hpp"
#include "QVectorMap.hpp"
#include "QCamSlider.hpp"


QCamAutoAlign::QCamAutoAlign() {
   tracker_=NULL;
   label(tr("Align"));
#if ONE_MAP
   shiftMap_=NULL;
#else
   shiftXhisto_=NULL;
   shiftYhisto_=NULL;
#endif
   scaleSlider_=NULL;
   findShiftCtrl_=NULL;
   findShiftWidget_=NULL;
   center_=false;
   cropValue_=0.8;
   fifoName_="/tmp/qastrocam-g2_shift.fifo";
   cout << "trying fifo "<<fifoName_<<"..."<<flush;
   fifo_=NULL;
   int fifoDescriptor=open(fifoName_.c_str(),O_RDWR|O_NONBLOCK);
   if (fifoDescriptor != -1) {
      struct stat fifoStats;
      fstat(fifoDescriptor,&fifoStats);
      if (S_ISFIFO(fifoStats.st_mode)) {
         fifo_=fdopen(fifoDescriptor,"r+");
         cout << "done"<<endl;
      } else {
         cout << "not a fifo"<<endl;
         close(fifoDescriptor);
      }
   } else {
      mkfifo(fifoName_.c_str(),S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
      fifoDescriptor=open(fifoName_.c_str(),O_RDWR|O_NONBLOCK);
      if(fifoDescriptor==-1) {
         perror(fifoName_.c_str());
      }
   }
}

QWidget* QCamAutoAlign::buildGUI(QWidget * parent) {
   QSizePolicy sizePolicyMax;
   sizePolicyMax.setVerticalPolicy(QSizePolicy::Expanding);
   sizePolicyMax.setHorizontalPolicy(QSizePolicy::Expanding);

   QSizePolicy sizePolicyMin;
   sizePolicyMin.setVerticalPolicy(QSizePolicy::Minimum);
   sizePolicyMin.setHorizontalPolicy(QSizePolicy::Minimum);

   QWidget* remoteCTRL= QCam::buildGUI(parent);

   QCamUtilities::registerWidget(remoteCTRL);

   QPushButton* resetCenter = new QPushButton(tr("reset"),remoteCTRL);
   connect(resetCenter,SIGNAL(pressed()),this,SLOT(reset()));
   findShiftWidget_=new QCamHGroupBox(tr("Find Shift Ctrl"),remoteCTRL);
   if (tracker_) {
      findShiftCtrl_=tracker_->buildGUI(findShiftWidget_);
   }

   cropSlider_=new QCamSlider(tr("crop"),false,remoteCTRL,1,100);
   cropSlider_->setSizePolicy(sizePolicyMin);
   connect(cropSlider_, SIGNAL(valueChange(int)),
           this,SLOT(setCropValue(int)));
   cropSlider_->setValue((int)round(cropValue_*100));
   cropSlider_->setToolTip(tr("% of image to keep when crooping"));
   centerButton_=new QCheckBox(tr("Center image"),remoteCTRL);
   connect(centerButton_,SIGNAL(toggled(bool)),this,SLOT(setImageCenter(bool)));
   centerButton_->setToolTip(tr("Center shifted images on the center of the frame"));

#if ONE_MAP
   scaleSlider_=new QCamSlider(tr("Display scale"),false,remoteCTRL,1,100);
   scaleSlider_->setSizePolicy(sizePolicyMin);
   scaleSlider_->setToolTip(tr("Scale of the shift history Map"));

   shiftMap_= new QVectorMap(remoteCTRL);
   shiftMap_->setSizePolicy(sizePolicyMax);
   shiftMap_->setMode(DrawLine);
   connect(scaleSlider_, SIGNAL(valueChange(int)),shiftMap_,SLOT(setScale(int)));
   shiftMap_->setToolTip(tr("Show the history of the frame shift"));
#else
   shiftXhisto_ = new QHistogram(remoteCTRL);
   shiftXhisto_->setDataSize(200);
   shiftXhisto_->setAutoShift(true);
   shiftXhisto_->setAverage(4);
   shiftYhisto_ = new QHistogram(remoteCTRL);
   shiftYhisto_->setDataSize(200);
   shiftYhisto_->setAutoShift(true);
   shiftYhisto_->setAverage(4);
#endif

   resetCenter->show();

   return remoteCTRL;
}

void QCamAutoAlign::reset() {
   if (tracker_) {
      tracker_->reset();
   }
#if ONE_MAP
   if (shiftMap_) {
      shiftMap_->reset();
   }
#endif
 }

const QSize & QCamAutoAlign::size() const {
   return yuvFrame_.size();
}

void QCamAutoAlign::resize(const QSize &s) {
   // do nothing
}

void QCamAutoAlign::setTracker(QCamFindShift * tracker) {
   unConnectEveryThing();
   tracker_=tracker;
   connectEveryThing();
}

void QCamAutoAlign::unConnectEveryThing() {
   if (tracker_) {
      disconnect(tracker_,SIGNAL(shift(const ShiftInfo & )),this,SLOT(shifted(const ShiftInfo & )));
      tracker_=NULL;
      if (findShiftCtrl_) {
         delete findShiftCtrl_;
         findShiftCtrl_=NULL;
      }
   }
}

bool QCamAutoAlign::connectEveryThing() {
   if (tracker_ == NULL) {
      return false;
   }
   tracker_->pause();
   connect(tracker_,SIGNAL(shift(const ShiftInfo & )),this,SLOT(shifted(const ShiftInfo & )));
   tracker_->resume();
   if (findShiftWidget_) {
      findShiftCtrl_=tracker_->buildGUI(findShiftWidget_);
   }
   return true;
}

void QCamAutoAlign::shifted(const ShiftInfo & shift) {
   if (false && currentSize_ != tracker_->cam().size()) {
      currentSize_=tracker_->cam().size();
      cout << "reset tracker: "
           <<currentSize_.width()<<"x"<<currentSize_.height()
           <<" "
           <<tracker_->cam().size().width()<<tracker_->cam().size().height()<<endl;
      reset();
   } else {
      currentShift_ = shift;
      shiftFrame(currentShift_,tracker_->cam().yuvFrame(),yuvFrame_,
                 cropValue_,center_);
#if ONE_MAP
      if (shiftMap_) shiftMap_->add(shift.shift());
#else
      if (shiftXhisto_) shiftXhisto_->setValue(shift.shift().x());
      if (shiftYhisto_) shiftYhisto_->setValue(shift.shift().y());
#endif
      importProperties(tracker_->cam());
      setProperty("shift X",shift.shift().x());
      setProperty("shift Y",shift.shift().y());
      setProperty("shift rotation",shift.angle());
      setProperty("Center X",shift.center().x());
      setProperty("Center Y",shift.center().y());
      if (fifo_) {
         int res=fprintf(fifo_,"%f %f\n",shift.shift().x(),shift.shift().y());
         if (res >= 0) {
            fflush(fifo_);
         } else {
            char buff[30];
            snprintf(buff,30,"writting fifo (%d)",res);
            perror(buff);
         }
      }
      newFrameAvaible();
   }
}

QCamFrame QCamAutoAlign::yuvFrame() const {
   return yuvFrame_;
}

void QCamAutoAlign::shiftFrame(const ShiftInfo & shift,const QCamFrame orig,
                               QCamFrame & shifted,float crop,bool center) {
   int shiftX=(int)shift.shift().x();
   int shiftY=(int)shift.shift().y();

   int cropX=(int)(orig.size().width()*(1.0-crop));
   int cropY=(int)(orig.size().height()*(1.0-crop));

   // field rotation not handled
   if (shift.angle() != 0) {
      cout << "Rotation not handled by QCamAutoAlign::shiftFrame().\n"
           << " Ignoring it\n";
   }
   if (shiftX == 0
       && shiftY == 0
       && shift.angle() == 0
       && crop >=1.0) {
      //no translation
      shifted = orig;
      return;
   }

   /* black background */
   shifted.setSize(QSize(orig.size().width()-cropX,
                         orig.size().height()-cropY));
   shifted.clear();
   shifted.setMode(orig.getMode());
   if (!center) {
      shifted.copy(orig,
                   shiftX+(cropX/2),shiftY+(cropY/2),
                   orig.size().width()+shiftX-(cropX/2),
                   orig.size().height()+shiftY-(cropY/2),
                   0,0);
   } else {
      int keepX=(orig.size().width()-cropX)/2;
      int keepY=(orig.size().height()-cropY)/2;
      shifted.copy(orig,
                   (int)round(shift.center().x()-keepX),
                   (int)round(shift.center().y()-keepY),
                   (int)round(shift.center().x()+keepX),
                   (int)round(shift.center().y()+keepY),
                   0,0);
   }
   //shifted.rotate(shifted.size().width()/2,shifted.size().height()/2,0.5);
   return;
}
