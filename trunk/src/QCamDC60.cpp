/******************************************************************
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

#include <sys/ioctl.h>
#include <math.h>

#include <linux/videodev2.h>

#include <Qt/qtooltip.h>
#include <Qt/qlabel.h>
#include <Qt/qprogressbar.h>

#include "dc60_private_ioctls.h"
#include "QCamDC60.hpp"
#include "SettingsBackup.hpp"

extern settingsBackup settings;

QCamDC60::QCamDC60(const char * devpath):
   QCamV4L2(devpath,ioNoBlock|ioUseSelect|haveBrightness|haveContrast|haveHue|haveColor) {
   if(settings.haveKey("LX_LEVELS_INVERTED")) {
      bool inverted_=(strcasecmp(settings.getKey("LX_LEVELS_INVERTED"),"yes")==0);
      if(inverted_)
         setInverted(true);
      else
         setInverted(false);
  }
}

void QCamDC60::setIntegration(int val) {
   struct v4l2_control ctrl;
   ctrl.id=V4L2_CID_INTEGRATION_TIME;
   ctrl.value=val;

   if(ioctl(device_,VIDIOC_S_CTRL,&ctrl)!=0)
      cout << "Unable to change DC60 integration time" << endl;
}

void QCamDC60::setInverted(bool b) {
   struct v4l2_control ctrl;
   ctrl.id=V4L2_CID_INTEGRATION_INVERTED;
   ctrl.value=b;

   if(ioctl(device_,VIDIOC_S_CTRL,&ctrl)!=0)
      cout << "Unable to change DC60 integration level state" << endl;
}

QWidget *  QCamDC60::buildGUI(QWidget * parent) {
   QWidget * remoteCTRL=QCamV4L2::buildGUI(parent);
   struct v4l2_control ctrl;

   // extra controls
   Q3VGroupBox* extraCtrl=new Q3VGroupBox("Extra controls",remoteCTRL);
   Q3HBox* line1=new Q3HBox(extraCtrl);
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
   Q3HGroupBox* lxCtrl=new Q3HGroupBox("Long exposure",remoteCTRL);
   QWidget* padding1=new QWidget(lxCtrl);
   lxCheck=new QCheckBox("Activate",lxCtrl);
   QWidget* padding2=new QWidget(lxCtrl);
   lxLabel=new QLabel("Time :",lxCtrl);
   lxLabel->setEnabled(false);
   lxEntry=new QLineEdit(lxCtrl);
   lxEntry->setMaximumWidth(48);
   lxEntry->setEnabled(false);
   lxEntry->setAlignment(Qt::AlignRight);
   lxEntry->setText("1");
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
   ctrl.value=(b==QCheckBox::On);
   if(ioctl(device_,VIDIOC_S_CTRL,&ctrl)!=0)
      extraPreamp->setEnabled(false);

   if(b==QCheckBox::On)
      extraAntialias->setEnabled(true);
   else {
      extraAntialias->setEnabled(false);
      antialiasChanged(false);
   }
}

void QCamDC60::antialiasChanged(int b) {
   struct v4l2_control ctrl;

   ctrl.id=V4L2_CID_ANTIALIAS;
   ctrl.value=(b==QCheckBox::On);
   if(ioctl(device_,VIDIOC_S_CTRL,&ctrl)!=0)
      extraAntialias->setEnabled(false);
}

void QCamDC60::whitepeakChanged(int b) {
   struct v4l2_control ctrl;

   ctrl.id=V4L2_CID_WHITEPEAK;
   ctrl.value=(b==QCheckBox::On);
   if(ioctl(device_,VIDIOC_S_CTRL,&ctrl)!=0)
      extraAntialias->setEnabled(false);
}

// lx slots

void QCamDC60::lxActivated(int b) {
   if(b==QCheckBox::On) {
      lxLabel->setEnabled(true);
      lxEntry->setEnabled(true);
      lxSet->setEnabled(true);
      lxProgress->setEnabled(true);
      // create objects
      progressTimer= new QTimer();
      // connect
      connect(progressTimer,SIGNAL(timeout()),this,SLOT(lxProgressStep()));
      lxSetPushed();
   } else {
      // delete objects
      delete progressTimer;
      lxLabel->setEnabled(false);
      lxEntry->setEnabled(false);
      lxSet->setEnabled(false);
      lxProgress->setEnabled(false);
      lxProgress->reset();
      setIntegration(0);
   }
}

void QCamDC60::lxSetPushed() {
   float val;
   // reading edit line and converts
   QString str=lxEntry->text();
   if (sscanf(str.latin1(),"%f",&val)!=1) {
      // default delay
      val=1;
   }
   // main delay
   if(val<1)
      val=1;
   // 1s steps
   val=round(val);
   lxDelay=val;
   // progress bar update
   lxProgress->setMinimum(0);
   lxProgress->setMaximum((int)(lxDelay));

   lxProgress->reset();
   // progress timer
   progressTimer->start(1000);
   progress=0;
   // text update
   lxEntry->setText(QString().sprintf("%4.0f",lxDelay));
   setProperty("FrameRateSecond",1.0/lxDelay);
   setIntegration((int)lxDelay);
}

void QCamDC60::lxProgressStep() {
   if(progress==lxProgress->maximum()) {
      progress=0;
      lxProgress->reset();
   } else {
      progress++;
      lxProgress->setValue(progress);
   }
}
