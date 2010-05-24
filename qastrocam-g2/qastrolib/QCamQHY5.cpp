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

#include <qmessagebox.h>
#include <qtimer.h>

#include "QCamQHY5.moc"

QCamQHY5::QCamQHY5() {
   // set cam label
   label(QString("QHY5"));
   // vars init
   sizeTable_=NULL;
   frameExposure_=200;
   frameRate_=1000/frameExposure_;
   xstart_=1280;
   ystart_=1024;
   width_=640;
   height_=480;
   gain_=50;
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
   timer_->start(frameExposure_);
   // set prop.
   static char buff[11];
   snprintf(buff,10,"%dx%d",width_,height_);
   setProperty("FrameSize",buff,true);
   setProperty("CameraName","QHY5");
   setProperty("FrameRateSecond",frameRate_);
   setProperty("Gain",gain_,false);
}

QCamQHY5::~QCamQHY5() {
   // read the last frame
   void* tmp=malloc(width_*height_);
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
   yuvBuffer_.setSize(QSize(width_,height_));
   setCam();
   static char buff[11];
   snprintf(buff,10,"%dx%d",x,y);
   setProperty("FrameSize",buff,true);
}

void QCamQHY5::setExposure(int e) {
   frameExposure_=e;
   frameRate_=1000/frameExposure_;
   timer_->start(frameExposure_) ;
   setProperty("FrameRateSecond",frameRate_);
}

void QCamQHY5::setGain(int g) {
   gain_=g;
   setCam();
   setProperty("Gain",gain_,false);
}

void QCamQHY5::setCam() {
   void* tmp=malloc(width_*height_);
   camera->read((char*)tmp);
   free(tmp);
   camera->configure(xstart_,ystart_,width_,height_,gain_,&width_,&height_);
   camera->shoot(frameExposure_);
}

const QSize & QCamQHY5::size() const {
   return yuvBuffer_.size();
}

QWidget * QCamQHY5::buildGUI(QWidget * parent) {
   QWidget * remoteCTRL=QCam::buildGUI(parent);
   return remoteCTRL;
}

bool QCamQHY5::updateFrame() {
   void* YBuff=NULL;
   YBuff=yuvBuffer_.YforOverwrite();
   camera->read((char*)YBuff);
   setTime();
   camera->shoot(frameExposure_);
   newFrameAvaible();
}
