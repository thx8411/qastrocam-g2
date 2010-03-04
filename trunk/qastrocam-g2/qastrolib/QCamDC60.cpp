/******************************************************************
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

#include <sys/ioctl.h>
#include <math.h>

#include <linux/videodev2.h>

#include <qtooltip.h>

#include "QCamDC60.moc"

#include "dc60_private_ioctls.h"

#define FRAMES_TO_DROP	1

QCamDC60::QCamDC60(const char * devpath):
   QCamV4L2(devpath,ioNoBlock|ioUseSelect|haveBrightness|haveContrast|haveHue|haveColor) {
   lxActiv=false;
   toDropBase=0;
   toDrop=toDropBase;
   toDropBuffer=FRAMES_TO_DROP;
   buffNumber=2;
}

void QCamDC60::startIntegration() {
   struct v4l2_control ctrl;
   ctrl.id=V4L2_CID_INTEGRATION_START;
   ctrl.value=true;

   if(ioctl(device_,VIDIOC_S_CTRL,&ctrl)!=0)
      cout << "Unable to change DC60 integration state" << endl;
}

void QCamDC60::stopIntegration() {
   struct v4l2_control ctrl;
   ctrl.id=V4L2_CID_INTEGRATION_STOP;
   ctrl.value=true;

   if(ioctl(device_,VIDIOC_S_CTRL,&ctrl)!=0)
      cout << "Unable to change DC60 integration state" << endl;
}

void QCamDC60::setInverted(bool b) {
   struct v4l2_control ctrl;
   ctrl.id=V4L2_CID_INTEGRATION_INVERTED;
   ctrl.value=b;

   if(ioctl(device_,VIDIOC_S_CTRL,&ctrl)!=0)
      cout << "Unable to change DC60 integration level state" << endl;
}

bool QCamDC60::updateFrame() {
   if(lxActiv) {
      if(toDrop==toDropBase)
         lxControler->startAccumulation();
      if(toDrop==0) {
         if(toDropBuffer==FRAMES_TO_DROP) {
            lxControler->stopAccumulation();
            lxProgress->reset();
         }
         if(toDropBuffer==0) {
            toDrop=toDropBase;
            toDropBuffer=FRAMES_TO_DROP;
           return(QCamV4L2::updateFrame());
         } else {
            toDropBuffer--;
            return(QCamV4L2::dropFrame());
            //return(QCamV4L2::updateFrame());
         }
      }
      toDrop--;
      lxProgress->setProgress(toDropBase-toDrop);
      return(QCamV4L2::dropFrame());
      //return(QCamV4L2::updateFrame());
   } else
   return(QCamV4L2::updateFrame());
}

QWidget *  QCamDC60::buildGUI(QWidget * parent) {
   QWidget * remoteCTRL=QCamV4L2::buildGUI(parent);
   struct v4l2_control ctrl;

   // extra controls
   QVGroupBox* extraCtrl=new QVGroupBox("Extra controls",remoteCTRL);
   QHBox* line1=new QHBox(extraCtrl);
   QWidget* padding5=new QWidget(line1);
   extraPreamp=new QCheckBox("Pre-amp",line1);
   QWidget* padding6=new QWidget(line1);
   extraAntialias=new QCheckBox("Anti-alias",line1);
   QWidget* padding7=new QWidget(line1);
   extraWhitepeak=new QCheckBox("White peak control",line1);
   QWidget* padding8=new QWidget(line1);

   // read values
   ctrl.id=V4L2_CID_PREAMP;
   ctrl.value=0;
   if(ioctl(device_,VIDIOC_G_CTRL,&ctrl)!=0)
      extraPreamp->setEnabled(false);
   extraPreamp->setChecked(ctrl.value!=0);

   ctrl.id=V4L2_CID_ANTIALIAS;
   ctrl.value=0;
   if(ioctl(device_,VIDIOC_G_CTRL,&ctrl)!=0)
      extraAntialias->setEnabled(false);
   extraAntialias->setChecked(ctrl.value!=0);
   if(!extraPreamp->isChecked())
      extraAntialias->setEnabled(false);

   ctrl.id=V4L2_CID_WHITEPEAK;
   ctrl.value=0;
   if(ioctl(device_,VIDIOC_G_CTRL,&ctrl)!=0)
      extraWhitepeak->setEnabled(false);
   extraWhitepeak->setChecked(ctrl.value!=0);

   // connects
   connect(extraPreamp,SIGNAL(stateChanged(int)),this,SLOT(preampChanged(int)));
   connect(extraAntialias,SIGNAL(stateChanged(int)),this,SLOT(antialiasChanged(int)));
   connect(extraWhitepeak,SIGNAL(stateChanged(int)),this,SLOT(whitepeakChanged(int)));

   // long exposure
   QHGroupBox* lxCtrl=new QHGroupBox("Long exposure",remoteCTRL);
   QWidget* padding1=new QWidget(lxCtrl);
   lxCheck=new QCheckBox("Activate",lxCtrl);
   QWidget* padding2=new QWidget(lxCtrl);
   lxLabel=new QLabel("Time :",lxCtrl);
   lxLabel->setEnabled(false);
   lxEntry=new QLineEdit(lxCtrl);
   lxEntry->setMaximumWidth(48);
   lxEntry->setEnabled(false);
   lxEntry->setText("0.2");
   lxSet=new QPushButton("Set",lxCtrl);
   lxSet->setMaximumWidth(32);
   lxSet->setEnabled(false);
   QWidget* padding3=new QWidget(lxCtrl);
   lxProgress= new QProgressBar(lxCtrl);
   lxProgress->setEnabled(false);
   QWidget* padding4=new QWidget(lxCtrl);

   QToolTip::add(lxCheck,"Activate long exposure mode");
   QToolTip::add(lxEntry,"Integration time value");
   QToolTip::add(lxSet,"Set integration time");
   QToolTip::add(lxProgress,"Integration progress");

   connect(lxCheck,SIGNAL(stateChanged(int)),this,SLOT(lxActivated(int)));
   connect(lxSet,SIGNAL(released()),this,SLOT(lxSetPushed()));

   return(remoteCTRL);
}


// gui slots

// extras slots

void QCamDC60::preampChanged(int b) {
   struct v4l2_control ctrl;

   ctrl.id=V4L2_CID_PREAMP;
   ctrl.value=(b==QButton::On);
   if(ioctl(device_,VIDIOC_S_CTRL,&ctrl)!=0)
      extraPreamp->setEnabled(false);

   if(b==QButton::On)
      extraAntialias->setEnabled(true);
   else {
      extraAntialias->setEnabled(false);
      antialiasChanged(false);
   }
}

void QCamDC60::antialiasChanged(int b) {
   struct v4l2_control ctrl;

   ctrl.id=V4L2_CID_ANTIALIAS;
   ctrl.value=(b==QButton::On);
   if(ioctl(device_,VIDIOC_S_CTRL,&ctrl)!=0)
      extraAntialias->setEnabled(false);
}

void QCamDC60::whitepeakChanged(int b) {
   struct v4l2_control ctrl;

   ctrl.id=V4L2_CID_WHITEPEAK;
   ctrl.value=(b==QButton::On);
   if(ioctl(device_,VIDIOC_S_CTRL,&ctrl)!=0)
      extraAntialias->setEnabled(false);
}

// lx slots

void QCamDC60::lxActivated(int b) {
   if(b==QButton::On) {
      lxLabel->setEnabled(true);
      lxEntry->setEnabled(true);
      lxSet->setEnabled(true);
      lxProgress->setEnabled(true);
      // create objects
      lxControler=new SCmodDC60(*this);
      lxSetPushed();
      lxActiv=true;
   } else {
      // stop lx
      // delete objects
      delete lxControler;
      lxLabel->setEnabled(false);
      lxEntry->setEnabled(false);
      lxSet->setEnabled(false);
      lxProgress->setEnabled(false);
      lxProgress->reset();
      lxActiv=false;
      toDropBase=0;
   }
}

void QCamDC60::lxSetPushed() {
   float val;
   lxControler->stopAccumulation();
   // reading edit line and converts
   QString str=lxEntry->text();
   if (sscanf(str.latin1(),"%f",&val)!=1) {
      // default delay
      val=0.2;
   }
   // main delay
   if(val<0.2)
      val=0.2;
   // 0.2s steps
   val=round(val*5)/5;
   lxDelay=val;
   // drop base
   toDropBase=(int)(lxDelay*frameRate_);
   toDrop=toDropBase;
   // progress bar update
   lxProgress->setTotalSteps(toDropBase);
   lxProgress->reset();
   // text update
   lxEntry->setText(QString().sprintf("%4.2f",lxDelay));
   setProperty("FrameRateSecond",1.0/lxDelay);
}
