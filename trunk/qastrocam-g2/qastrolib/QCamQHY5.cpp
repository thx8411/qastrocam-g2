/******************************************************************
Qastrocam-g2
Copyright (C) 2010   Blaise-Florentin Collin

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

#include <qmessagebox.h>
#include <qtimer.h>
#include <qvgroupbox.h>
#include <qhbox.h>

#include "QCamQHY5.moc"

#define PROGRESS_TIME	200

int QCamQHY5::getTime(int v) {
   return((int)(exp((float)v/10.0-2.0)*1000.0));
}

QCamQHY5::QCamQHY5() {
   // set cam label
   label(QString("QHY5"));
   // vars init
   sizeTable_=NULL;
   frameExposure_=getTime(0);
   frameRate_=1000/frameExposure_;
   xstart_=0;
   ystart_=0;
   width_=640;
   height_=480;
   gain_=10;
   //sizeTable=getAllowedSize();
   // get the cam instance
   camera=QHY5cam::instance(QHY_IMAGER);
   if(camera==NULL) {
      QMessageBox::information(0,"Qastrocam-g2","Unable to reach the QHY5 imager\nLeaving...");
      exit(1);
   }
   // configure the cam
   camera->configure(xstart_,ystart_,width_,height_,gain_,&width_,&height_);
   // set frame
   yuvBuffer_.setMode(GreyFrame);
   yuvBuffer_.setSize(QSize(width_,height_));
   // start the first frame
   camera->shoot(frameExposure_);
   // set the timer
   timer_=new QTimer(this);
   connect(timer_,SIGNAL(timeout()),this,SLOT(updateFrame()));
   timer_->start(frameExposure_,true);
   // set prop.
   static char buff[11];
   snprintf(buff,10,"%dx%d",width_,height_);
   setProperty("FrameSize",buff,true);
   setProperty("CameraName","QHY5");
   setProperty("FrameRateSecond",frameRate_);
   setProperty("Gain",gain_,false);
}

QCamQHY5::~QCamQHY5() {
   void* tmp;
   // read the last frame
   tmp=malloc(width_*height_);
   camera->read((char*)tmp);
   free(tmp);
   QHY5cam::destroy(QHY_IMAGER);
}

void QCamQHY5::resize(const QSize & s) {
   setSize(s.width(),s.height());
}

const QSize * QCamQHY5::getAllowedSize() const {
   if (sizeTable_==NULL) {
      sizeTable_=new QSize[5];
      int currentIndex=0;
      sizeTable_[currentIndex++]=QSize(1280,1024);
      sizeTable_[currentIndex++]=QSize(640,480);
      sizeTable_[currentIndex++]=QSize(320,240);
      sizeTable_[currentIndex++]=QSize(160,120);
      sizeTable_[currentIndex++]=QSize(0,0);
   }
   return sizeTable_;
}

void QCamQHY5::setSize(int x, int y) {
   width_=x;
   height_=y;
   xstart_=(1280-x)/2;
   ystart_=(1024-y)/2;
   // drop the last frame
   void* YBuff=NULL;
   YBuff=yuvBuffer_.YforOverwrite();
   camera->read((char*)YBuff);
   yuvBuffer_.setSize(QSize(width_,height_));
   // start the new frame
   camera->configure(xstart_,ystart_,width_,height_,gain_,&width_,&height_);
   camera->shoot(frameExposure_);
   // update datas
   static char buff[11];
   snprintf(buff,10,"%dx%d",x,y);
   setProperty("FrameSize",buff,true);
}

void QCamQHY5::setExposure() {
   frameRate_=1000/frameExposure_;
   timer_->start(frameExposure_,true);
   setProperty("FrameRateSecond",frameRate_);
   if(frameExposure_>(3*PROGRESS_TIME)) {
      progressBar->setEnabled(true);
      progressBar->setTotalSteps((int)((float)(frameExposure_+30)/PROGRESS_TIME));
      progressBar->reset();
      progress_=0;
   } else {
      progressBar->reset();
      progressBar->setEnabled(false);
   }
}

void QCamQHY5::setGain() {
   setProperty("Gain",gain_,false);
}

void QCamQHY5::changeExposure(int e) {
   frameExposure_=getTime(e);
   exposureValue->setText(QString().sprintf("%6.2f",(float)frameExposure_/1000));
}

void QCamQHY5::changeGain(int g) {
   gain_=g;
}

const QSize & QCamQHY5::size() const {
   return yuvBuffer_.size();
}

QWidget * QCamQHY5::buildGUI(QWidget * parent) {
   QWidget* remoteCTRL=QCam::buildGUI(parent);
   QVGroupBox* settingsBox=new QVGroupBox(QString("Settings"),remoteCTRL);
   // gain
   gainSlider=new QCamSlider("Gain",false,settingsBox);
   gainSlider->setMinValue(0);
   gainSlider->setMaxValue(100);
   gainSlider->setValue(gain_);
   // exposure
   QHBox* exposureBox=new QHBox(settingsBox);
   QLabel* label1=new QLabel(QString("Exposure"),exposureBox);
   exposureSlider=new QSlider(Qt::Horizontal,exposureBox);
   exposureSlider->setMinValue(0);
   exposureSlider->setMaxValue(50);
   exposureValue=new QLabel(exposureBox);
   exposureValue->setText(QString().sprintf("%6.2f",(float)getTime(0)/1000));
   QLabel* label2=new QLabel(QString("s"),exposureBox);
   // progress bar
   QHBox* progressBox=new QHBox(settingsBox);
   QLabel* label3=new QLabel(QString("Progress"),progressBox);
   progressBar=new QProgressBar(progressBox);
   if(frameExposure_>(3*PROGRESS_TIME)) {
      progressBar->setEnabled(true);
      progressBar->setTotalSteps((int)((float)(frameExposure_+30)/PROGRESS_TIME));
      progress_=0;
   } else {
      progressBar->setEnabled(false);
   }
   // connections
   connect(gainSlider,SIGNAL(valueChange(int)),this,SLOT(changeGain(int)));
   connect(gainSlider,SIGNAL(sliderReleased()),this,SLOT(setGain()));
   connect(exposureSlider,SIGNAL(valueChanged(int)),this,SLOT(changeExposure(int)));
   connect(exposureSlider,SIGNAL(sliderReleased()),this,SLOT(setExposure()));
   // progress timer
   progressTimer_=new QTimer(this);
   progressTimer_->start(PROGRESS_TIME);
   connect(progressTimer_,SIGNAL(timeout()),this,SLOT(progressUpdate()));

   return remoteCTRL;
}

void QCamQHY5::progressUpdate() {
   if(frameExposure_>(3*PROGRESS_TIME)) {
      progress_++;
      progressBar->setProgress(progress_);
   }
}

bool QCamQHY5::updateFrame() {
   void* YBuff=NULL;
   YBuff=yuvBuffer_.YforOverwrite();
   if(camera->read((char*)YBuff)) {
      setTime();
      camera->configure(xstart_,ystart_,width_,height_,gain_,&width_,&height_);
      camera->shoot(frameExposure_);
      timer_->start(frameExposure_,true);
      newFrameAvaible();
      if(frameExposure_>(3*PROGRESS_TIME)) {
         progressBar->reset();
         progress_=0;
      }
   }
}
