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
#include <qtooltip.h>

#include "SettingsBackup.hpp"

#include "QCamQHY6.moc"

// camera's pixel rate (per ms)
#define PIXEL_RATE	24000

// the GUI can't handle high frame rates
#define PROGRESS_TIME	200

// settings object, needed everywhere
extern settingsBackup settings;

//
const int QCamQHY6::exposureTable[QHY6_EXPOSURE_TABLE_SIZE]={20,40,50,66,100,200,1000,1500,2000,3000,4000,5000,10000,15000,20000,30000,60000};

// the exposure slider use a table
// returns exposure time in ms
int QCamQHY6::getExposureTime(int i) {
   return(exposureTable[i]);
}

// return the given time in ms index
int QCamQHY6::getExposureIndex(int t) {
   int index=0;
   while((t>exposureTable[index])&&(index<QHY6_EXPOSURE_TABLE_SIZE))
      index++;
   if(index==QHY6_EXPOSURE_TABLE_SIZE)
      index--;
   return(index);
}

QCamQHY6::QCamQHY6() {
   // set cam label
   label(QString("QHY6"));
   // vars init
   sizeTable_=NULL;
   exposureValue=NULL;

   // message
   cerr << "Starting QHY6, please wait..." << endl;

   // setting exposure
   if(settings.haveKey("QHY6_EXPOSURE")) {
      frameExposure_=atoi(settings.getKey("QHY6_EXPOSURE"));
      if(frameExposure_==0)
         frameExposure_=getExposureTime(0);
   } else
      frameExposure_=getExposureTime(0);

   width_=796;
   height_=596;
   targetWidth_=width_;
   targetHeight_=height_;
   targetSize.setWidth(targetWidth_);
   targetSize.setHeight(targetHeight_);

   // setting gains
   if(settings.haveKey("QHY6_GAIN")) {
      gain_=atoi(settings.getKey("QHY6_GAIN"));
      if(gain_==0) gain_=5;
   } else
      gain_=5;

   sizeTable=getAllowedSize();
   // get the cam instance
   camera=QHY6cam::instance(QHY6_IMAGER);
   if(camera==NULL) {
      QMessageBox::information(0,"Qastrocam-g2","Unable to reach the QHY6 imager\nLeaving...");
      exit(1);
   }

   // set frame
   inputBuffer_.setMode(GreyFrame);
   inputBuffer_.setSize(QSize(width_,height_));
   // configure the cam
   camera->configure(frameExposure_,gain_);
   // set prop.
   static char buff[11];
   snprintf(buff,10,"%dx%d",width_,height_);
   setProperty("FrameSize",buff,true);
   setProperty("CameraName","QHY6");
   setProperty("FrameExposure",frameExposure_);
   setProperty("Gain",gain_,false);
}

QCamQHY6::~QCamQHY6() {
   void* tmp;

   cerr << "Closing QHY6, please wait..." << endl;

   // read the last frame
   camera->stop();
   tmp=malloc(width_*height_);
   // message
   if(frameExposure_>5000) {
      QMessageBox::information(0,"Qastrocam-g2","Sorry, we must wait for the last frame to be read...");
   }
   // last frame read
   camera->read((char*)tmp,shootMode_);
   free(tmp);
   // release the imager
   QHY6cam::destroy(QHY6_IMAGER);

   cerr << "QHY6 closed..." << endl;
}

void QCamQHY6::resize(const QSize & s) {
   setSize(s.width(),s.height());
}

const QSize * QCamQHY6::getAllowedSize() const {
   // lists sizes
   if (sizeTable_==NULL) {
      sizeTable_=new QSize[5];
      int currentIndex=0;
      sizeTable_[currentIndex++]=QSize(796,596);
      sizeTable_[currentIndex++]=QSize(398,298);
      sizeTable_[currentIndex++]=QSize(0,0);
   }
   return sizeTable_;
}

void QCamQHY6::setSize(int x, int y) {
   // drop the last frame
   void* YBuff=NULL;
   camera->stop();
   YBuff=inputBuffer_.YforOverwrite();
   camera->read((char*)YBuff,shootMode_);

   // selects resizing mode
   switch(croppingMode) {
      case BINNING :
      case SCALING :
      case CROPPING :
         // update vars
         width_=796;
         height_=596;
         targetWidth_=x;
         targetHeight_=y;
         break;
   }
   // update size
   targetSize.setWidth(targetWidth_);
   targetSize.setHeight(targetHeight_);
   // update frame mem
   inputBuffer_.setSize(QSize(width_,height_));
   // count the usb transfer time. Rate is 24M pixels / second
   shootMode_=(frameExposure_<1000);
   int poseTime=frameExposure_-(1558*(height_+26)/PIXEL_RATE);
   if(poseTime<0) poseTime=0;
   // start the new frame
   camera->configure(frameExposure_,gain_);
   camera->shoot(poseTime,shootMode_);
   // update datas
   static char buff[11];
   snprintf(buff,10,"%dx%d",x,y);
   setProperty("FrameSize",buff,true);

   // update display
   if(exposureValue!=NULL) {
      int transferTime=1558*(height_+26)/PIXEL_RATE;
      if(transferTime>frameExposure_) {
         exposureValue->setText(QString().sprintf("%2i fps (max)",(int)(1.0/(float)transferTime*1000)));
      } else {
         if(frameExposure_<1000)
            exposureValue->setText(QString().sprintf("%2i fps",(int)(1.0/(float)frameExposure_*1000)));
         else
            exposureValue->setText(QString().sprintf("%2.1f s",((float)frameExposure_/1000)));
      }
   }
}

void QCamQHY6::setExposure() {
   char value[10];

   // resets the cam
   camera->stop();
   timer_->start(frameExposure_,true);
   // update conf file
   sprintf(value,"%i",frameExposure_);
   settings.setKey("QHY6_EXPOSURE",value);
   // update properties
   setProperty("FrameExposure",frameExposure_);
   // disable progress bar for short time
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

void QCamQHY6::setGain() {
   char value[4];
   setProperty("Gain",gain_,false);
   sprintf(value,"%i",gain_);
   settings.setKey("QHY6_GAIN",value);
   sprintf(value,"%i",gain_);
}

void QCamQHY6::changeExposure(int e) {
   // update exposure time
   frameExposure_=getExposureTime(e);

   // update display
   int transferTime=1558*(height_+26)/PIXEL_RATE;
   if(transferTime>frameExposure_) {
      exposureValue->setText(QString().sprintf("%2i fps (max)",(int)(1.0/(float)transferTime*1000)));
   } else {
      if(frameExposure_<1000)
         exposureValue->setText(QString().sprintf("%2i fps",(int)(1.0/(float)frameExposure_*1000)));
      else
         exposureValue->setText(QString().sprintf("%2.1f s",((float)frameExposure_/1000)));
   }
}

void QCamQHY6::changeGain(int g) {
   gain_=g;
}

const QSize & QCamQHY6::size() const {
   return(targetSize);
}

QWidget * QCamQHY6::buildGUI(QWidget * parent) {
   QWidget* remoteCTRL=QCam::buildGUI(parent);
   QVGroupBox* settingsBox=new QVGroupBox(QString("Settings"),remoteCTRL);

   // gain
   gainSlider=new QCamSlider("Gain",false,settingsBox,0,63,false,false);
   gainSlider->setValue(gain_);

   // exposure
   QHBox* exposureBox=new QHBox(settingsBox);
   QLabel* label1=new QLabel(QString("Exposure"),exposureBox);
   exposureSlider=new QSlider(Qt::Horizontal,exposureBox);
   exposureSlider->setMinValue(0);
   exposureSlider->setMaxValue(QHY6_EXPOSURE_TABLE_SIZE-1);
   exposureSlider->setValue(getExposureIndex(frameExposure_));
   exposureSlider->setTickmarks(QSlider::Below);
   exposureSlider->setTickInterval(1);
   exposureValue=new QLabel(exposureBox);
   exposureValue->setMinimumWidth(80);
   // update value
   int transferTime=1558*(height_+26)/PIXEL_RATE;
   if(transferTime>frameExposure_) {
      exposureValue->setText(QString().sprintf("%2i fps (max)",(int)(1.0/(float)transferTime*1000)));
   } else {
      if(frameExposure_<1000)
         exposureValue->setText(QString().sprintf("%2i fps",(int)(1.0/(float)frameExposure_*1000)));
      else
         exposureValue->setText(QString().sprintf("%2.1f s",((float)frameExposure_/1000)));
   }
   // if exposure > 1000ms, read the frame and start a new one
   // count the usb transfer time. Rate is 24M pixels / second
   shootMode_=(frameExposure_<1000);
   int poseTime=frameExposure_-(1558*(height_+26)/PIXEL_RATE);
   if(poseTime<0) poseTime=0;
   camera->stop();
   camera->shoot(poseTime,shootMode_);

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

   // tooltips
   QToolTip::add(gainSlider,tr("Camera's gain"));
   QToolTip::add(exposureSlider,tr("Camera's exposure"));
   QToolTip::add(exposureValue,tr("Exposure time"));

   // connections
   connect(gainSlider,SIGNAL(valueChange(int)),this,SLOT(changeGain(int)));
   connect(gainSlider,SIGNAL(sliderReleased()),this,SLOT(setGain()));

   connect(exposureSlider,SIGNAL(valueChanged(int)),this,SLOT(changeExposure(int)));
   connect(exposureSlider,SIGNAL(sliderReleased()),this,SLOT(setExposure()));

   cerr << "QHY6 ready..." << endl;

   // set the first timer shot
   timer_=new QTimer(this);
   connect(timer_,SIGNAL(timeout()),this,SLOT(updateFrame()));
   timer_->start(/*frameRate_*/frameExposure_,true);
   // progress timer
   progressTimer_=new QTimer(this);
   progressTimer_->start(PROGRESS_TIME);
   connect(progressTimer_,SIGNAL(timeout()),this,SLOT(progressUpdate()));

   return remoteCTRL;
}

void QCamQHY6::progressUpdate() {
   // update progress bar (only if needed)
   if(frameExposure_>(3*PROGRESS_TIME)) {
      progress_++;
      progressBar->setProgress(progress_);
   }
}

bool QCamQHY6::updateFrame() {
   // get the frame buffer
   void* YBuff=NULL;
   YBuff=inputBuffer_.YforOverwrite();
   // read picture datas
   if(camera->read((char*)YBuff,shootMode_)) {
      setTime();
      camera->configure(frameExposure_,gain_);
      // count the usb transfer time. Rate is 24M pixels / second
      shootMode_=(frameExposure_<1000);
      int poseTime=frameExposure_-(1558*(height_+26)/PIXEL_RATE);
      if(poseTime<0) poseTime=0;
      camera->shoot(poseTime,shootMode_);
      // gives a new shot for the timer
      timer_->start(frameExposure_,true);
      // set the output frame
      if((targetWidth_==796)&&(targetHeight_==596)) {
          // nothing to resize
          yuvBuffer_=inputBuffer_;
      } else {
         switch(croppingMode) {
            case SCALING :
               yuvBuffer_.scaling(inputBuffer_,targetWidth_,targetHeight_);
               break;
            case CROPPING :
               yuvBuffer_.cropping(inputBuffer_,(796-targetWidth_)/2,(596-targetHeight_)/2,targetWidth_,targetHeight_);
               break;
            case BINNING :
               yuvBuffer_.binning(inputBuffer_,targetWidth_,targetHeight_);
               break;
         }
      }
      // publish the frame
      newFrameAvaible();
      // update progress bar if needed
      if(frameExposure_>(3*PROGRESS_TIME)) {
         progressBar->reset();
         progress_=0;
      }
   }
}
