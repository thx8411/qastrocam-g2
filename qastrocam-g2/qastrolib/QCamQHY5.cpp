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

#include "QCamQHY5.moc"

// camera's pixel rate (per ms)
#define PIXEL_RATE	24000

// the GUI can't handle high frame rates
#define PROGRESS_TIME	200

// settings object, needed everywhere
extern settingsBackup settings;

//
const int QCamQHY5::exposureTable[QHY5_EXPOSURE_TABLE_SIZE]={20,40,50,66,100,200,1000,1500,2000,3000,4000,5000,10000,15000,20000,30000,60000};

// the exposure slider use a table
// returns exposure time in ms
int QCamQHY5::getExposureTime(int i) {
   return(exposureTable[i]);
}

// return the given time in ms index
int QCamQHY5::getExposureIndex(int t) {
   int index=0;
   while((t>exposureTable[index])&&(index<QHY5_EXPOSURE_TABLE_SIZE))
      index++;
   if(index==QHY5_EXPOSURE_TABLE_SIZE)
      index--;
   return(index);
}

QCamQHY5::QCamQHY5() {
   // set cam label
   label(QString("QHY5"));
   // vars init
   sizeTable_=NULL;
   exposureValue=NULL;

   // message
   cerr << "Starting QHY5, please wait..." << endl;

   // setting exposure
   if(settings.haveKey("QHY5_EXPOSURE")) {
      frameExposure_=atoi(settings.getKey("QHY5_EXPOSURE"));
      if(frameExposure_==0)
         frameExposure_=getExposureTime(0);
   } else
      frameExposure_=getExposureTime(0);
   //frameRate_=frameExposure_;
   //if(frameRate_<PROGRESS_TIME) frameRate_=PROGRESS_TIME;
   //frameRate_=PROGRESS_TIME;

   xstart_=0;
   ystart_=0;
   width_=640;
   height_=512;
   targetWidth_=width_;
   targetHeight_=height_;
   targetSize.setWidth(targetWidth_);
   targetSize.setHeight(targetHeight_);

   // setting gains
   if(settings.haveKey("QHY5_GAIN_G1")) {
      gainG1_=atoi(settings.getKey("QHY5_GAIN_G1"));
      if(gainG1_==0) gainG1_=37;
   } else
      gainG1_=37;
   if(settings.haveKey("QHY5_GAIN_G2")) {
      gainG2_=atoi(settings.getKey("QHY5_GAIN_G2"));
      if(gainG2_==0) gainG2_=37;
   } else
      gainG2_=37;
   if(settings.haveKey("QHY5_GAIN_R")) {
      gainR_=atoi(settings.getKey("QHY5_GAIN_R"));
      if(gainR_==0) gainR_=37;
   } else
      gainR_=37;
   if(settings.haveKey("QHY5_GAIN_B")) {
      gainB_=atoi(settings.getKey("QHY5_GAIN_B"));
      if(gainB_==0) gainB_=37;
   } else
      gainB_=37;

   sizeTable=getAllowedSize();
   // get the cam instance
   camera=QHY5cam::instance(QHY_IMAGER);
   if(camera==NULL) {
      QMessageBox::information(0,"Qastrocam-g2","Unable to reach the QHY5 imager\nLeaving...");
      exit(1);
   }

   // set frame
   inputBuffer_.setMode(GreyFrame);
   inputBuffer_.setSize(QSize(width_,height_));
   // configure the cam
   camera->configure(xstart_,ystart_,width_,height_,gainG1_,gainB_,gainR_,gainG2_,&width_,&height_);
   // start the first frame
   // count the usb transfer time. Rate is 24M pixels / second
   shootMode_=(frameExposure_<1000);
   int poseTime=frameExposure_-(1558*(height_+26)/PIXEL_RATE);
   if(poseTime<0) poseTime=0;
   camera->shoot(poseTime,shootMode_);
   // set the first timer shot
   timer_=new QTimer(this);
   connect(timer_,SIGNAL(timeout()),this,SLOT(updateFrame()));
   timer_->start(/*frameRate_*/frameExposure_,true);
   // set prop.
   static char buff[11];
   snprintf(buff,10,"%dx%d",width_,height_);
   setProperty("FrameSize",buff,true);
   setProperty("CameraName","QHY5");
   setProperty("FrameExposure",frameExposure_);
   setProperty("Gain Green1",gainG1_,false);
   setProperty("Gain Green2",gainG1_,false);
   setProperty("Gain Red",gainR_,false);
   setProperty("Gain Blue",gainB_,false);
}

QCamQHY5::~QCamQHY5() {
   void* tmp;

   cerr << "Closing QHY5, please wait..." << endl;

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
   QHY5cam::destroy(QHY_IMAGER);

   cerr << "QHY5 closed..." << endl;
}

void QCamQHY5::resize(const QSize & s) {
   setSize(s.width(),s.height());
}

const QSize * QCamQHY5::getAllowedSize() const {
   // lists sizes
   if (sizeTable_==NULL) {
      sizeTable_=new QSize[5];
      int currentIndex=0;
      sizeTable_[currentIndex++]=QSize(1280,1024);
      sizeTable_[currentIndex++]=QSize(640,512);
      sizeTable_[currentIndex++]=QSize(320,256);
      sizeTable_[currentIndex++]=QSize(0,0);
   }
   return sizeTable_;
}

void QCamQHY5::setSize(int x, int y) {
   // drop the last frame
   //void* YBuff=NULL;
   //camera->stop();
   //YBuff=inputBuffer_.YforOverwrite();
   //camera->read((char*)YBuff,shootMode_);

   // selects resizing mode
   switch(croppingMode) {
      case BINNING :
      case SCALING :
         // update vars
         width_=1280;
         height_=1024;
         targetWidth_=x;
         targetHeight_=y;
         xstart_=0;
         ystart_=0;
         break;
      case CROPPING :
         // update vars
         width_=x;
         height_=y;
         targetWidth_=width_;
         targetHeight_=height_;
         xstart_=(1280-x)/2;
         ystart_=(1024-y)/2;
         break;
   }
   // update size
   targetSize.setWidth(targetWidth_);
   targetSize.setHeight(targetHeight_);
   // update frame mem
   inputBuffer_.setSize(QSize(width_,height_));
   // start the new frame
   camera->configure(xstart_,ystart_,width_,height_,gainG1_,gainB_,gainR_,gainG2_,&width_,&height_);
   // count the usb transfer time. Rate is 24M pixels / second
   shootMode_=(frameExposure_<1000);
   int poseTime=frameExposure_-(1558*(height_+26)/PIXEL_RATE);
   if(poseTime<0) poseTime=0;
   camera->stop();
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

void QCamQHY5::setExposure() {
   char value[10];

   // resets the cam
   camera->stop();
   // update vars
   //frameRate_=frameExposure_;
   //if(frameRate_<PROGRESS_TIME) frameRate_=PROGRESS_TIME;
   //frameRate_=PROGRESS_TIME;
   timer_->start(/*frameRate_*/frameExposure_,true);
   // update conf file
   sprintf(value,"%i",frameExposure_);
   settings.setKey("QHY5_EXPOSURE",value);
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

void QCamQHY5::setGain() {
   char value[4];
   setProperty("Gain Green1",gainG1_,false);
   setProperty("Gain Green2",gainG1_,false);
   setProperty("Gain Red",gainR_,false);
   setProperty("Gain Blue",gainB_,false);
   sprintf(value,"%i",gainG1_);
   settings.setKey("QHY5_GAIN_G1",value);
   sprintf(value,"%i",gainG2_);
   settings.setKey("QHY5_GAIN_G2",value);
   sprintf(value,"%i",gainR_);
   settings.setKey("QHY5_GAIN_R",value);
   sprintf(value,"%i",gainB_);
   settings.setKey("QHY5_GAIN_B",value);
}

void QCamQHY5::changeExposure(int e) {
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

void QCamQHY5::changeGain(int g) {
   gainG1_=g;
   gainG2_=g;
   gainR_=g;
   gainB_=g;
}

void QCamQHY5::changeGainG1(int g) {
   gainG1_=g;
}

void QCamQHY5::changeGainG2(int g) {
   gainG2_=g;
}

void QCamQHY5::changeGainR(int g) {
   gainR_=g;
}

void QCamQHY5::changeGainB(int g) {
   gainB_=g;
}

const QSize & QCamQHY5::size() const {
   return(targetSize);
}

QWidget * QCamQHY5::buildGUI(QWidget * parent) {
   QWidget* remoteCTRL=QCam::buildGUI(parent);
   QVGroupBox* settingsBox=new QVGroupBox(QString("Settings"),remoteCTRL);

   // gain
   gainSlider=new QCamSlider("Gain",false,settingsBox);
   gainSlider->setMinValue(1);
   gainSlider->setMaxValue(73);
   gainSlider->setValue(gainG1_);

   gainSliderG1=new QCamSlider("Gain Green 1",false,settingsBox);
   gainSliderG1->setMinValue(1);
   gainSliderG1->setMaxValue(73);
   gainSliderG1->setValue(gainG1_);

   gainSliderG2=new QCamSlider("Gain Green 2",false,settingsBox);
   gainSliderG2->setMinValue(1);
   gainSliderG2->setMaxValue(73);
   gainSliderG2->setValue(gainG2_);

   gainSliderR=new QCamSlider("Gain Red",false,settingsBox);
   gainSliderR->setMinValue(1);
   gainSliderR->setMaxValue(73);
   gainSliderR->setValue(gainR_);

   gainSliderB=new QCamSlider("Gain Blue",false,settingsBox);
   gainSliderB->setMinValue(1);
   gainSliderB->setMaxValue(73);
   gainSliderB->setValue(gainB_);

   // no finished at this time, so hidden
   gainSliderG1->hide();
   gainSliderG2->hide();
   gainSliderR->hide();
   gainSliderB->hide();

   // exposure
   QHBox* exposureBox=new QHBox(settingsBox);
   QLabel* label1=new QLabel(QString("Exposure"),exposureBox);
   exposureSlider=new QSlider(Qt::Horizontal,exposureBox);
   exposureSlider->setMinValue(0);
   exposureSlider->setMaxValue(QHY5_EXPOSURE_TABLE_SIZE-1);
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
   QToolTip::add(gainSlider,tr("Camera's gain "));
   QToolTip::add(exposureSlider,tr("Camera's exposure, may be limited by the frame sizes"));
   QToolTip::add(exposureValue,tr("Exposure time (not a real fps, not very accurate for high rates)"));

   // connections
   connect(gainSlider,SIGNAL(valueChange(int)),this,SLOT(changeGain(int)));
   connect(gainSlider,SIGNAL(sliderReleased()),this,SLOT(setGain()));
   connect(gainSliderG1,SIGNAL(valueChange(int)),this,SLOT(changeGainG1(int)));
   connect(gainSliderG1,SIGNAL(sliderReleased()),this,SLOT(setGain()));
   connect(gainSliderG2,SIGNAL(valueChange(int)),this,SLOT(changeGainG2(int)));
   connect(gainSliderG2,SIGNAL(sliderReleased()),this,SLOT(setGain()));
   connect(gainSliderR,SIGNAL(valueChange(int)),this,SLOT(changeGainR(int)));
   connect(gainSliderR,SIGNAL(sliderReleased()),this,SLOT(setGain()));
   connect(gainSliderB,SIGNAL(valueChange(int)),this,SLOT(changeGainB(int)));
   connect(gainSliderB,SIGNAL(sliderReleased()),this,SLOT(setGain()));

   connect(exposureSlider,SIGNAL(valueChanged(int)),this,SLOT(changeExposure(int)));
   connect(exposureSlider,SIGNAL(sliderReleased()),this,SLOT(setExposure()));
   // progress timer
   progressTimer_=new QTimer(this);
   progressTimer_->start(PROGRESS_TIME);
   connect(progressTimer_,SIGNAL(timeout()),this,SLOT(progressUpdate()));

   cerr << "QHY5 ready..." << endl;

   return remoteCTRL;
}

void QCamQHY5::progressUpdate() {
   // update progress bar (only if needed)
   if(frameExposure_>(3*PROGRESS_TIME)) {
      progress_++;
      progressBar->setProgress(progress_);
   }
}

bool QCamQHY5::updateFrame() {
   // get the frame buffer
   void* YBuff=NULL;
   YBuff=inputBuffer_.YforOverwrite();
   // read picture datas
   if(camera->read((char*)YBuff,shootMode_)) {
      setTime();
      camera->configure(xstart_,ystart_,width_,height_,gainG1_,gainB_,gainR_,gainG2_,&width_,&height_);
      // count the usb transfer time. Rate is 24M pixels / second
      shootMode_=(frameExposure_<1000);
      int poseTime=frameExposure_-(1558*(height_+26)/PIXEL_RATE);
      if(poseTime<0) poseTime=0;
      camera->shoot(poseTime,shootMode_);
      // gives a new shot for the timer
      timer_->start(/*frameRate_*/frameExposure_,true);
      // set the output frame
      if((targetWidth_==1280)&&(targetHeight_==1024)) {
          // nothing to resize
          yuvBuffer_=inputBuffer_;
      } else {
         switch(croppingMode) {
            case SCALING :
               yuvBuffer_.scaling(inputBuffer_,targetWidth_,targetHeight_);
               break;
            case CROPPING :
               // cropping allready done by driver
               yuvBuffer_=inputBuffer_;
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
