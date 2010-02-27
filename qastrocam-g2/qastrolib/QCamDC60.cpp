/******************************************************************
Qastrocam
Copyright (C) 2003-2009   Franck Sicard
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

#include <qtooltip.h>

#include "QCamDC60.moc"

#include "private_ioctl.h"

QCamDC60::QCamDC60(const char * devpath):
   QCamV4L2(devpath,ioNoBlock|ioUseSelect|haveBrightness|haveContrast|haveHue|haveColor) {
}

void QCamDC60::setGPSW(bool b) {
   struct v4l2_control ctrl;
   ctrl.id=V4L2_CID_GPSW1;
   ctrl.value=b;

   if(ioctl(device_,VIDIOC_S_CTRL,&ctrl)!=0)
      cout << "Unable to change GPSW state" << endl;
}

QWidget *  QCamDC60::buildGUI(QWidget * parent) {
   QWidget * remoteCTRL=QCamV4L2::buildGUI(parent);
   remoteCTRLlx->hide();

   // extra controls
   QVGroupBox* extraCtrl=new QVGroupBox("Extra controls",remoteCTRL);

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
   lxEntry->setText("0.02");
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

void QCamDC60::lxActivated(int b) {
   if(b==QButton::On) {
      lxTimer= new QTimer();
      progressTimer= new QTimer();
      lxLabel->setEnabled(true);
      lxEntry->setEnabled(true);
      lxSet->setEnabled(true);
      lxProgress->setEnabled(true);
      // create objects
      lxTimer= new QTimer();
      progressTimer= new QTimer();
      lxControler=new SCmodDC60(*this);
      // connect
      connect(progressTimer,SIGNAL(timeout()),this,SLOT(lxProgressStep()));
      connect(lxTimer,SIGNAL(timeout()),this,SLOT(lxTimeout()));
   } else {
      // stop lx
      // delete objects
      delete lxControler;
      delete progressTimer;
      delete lxTimer;
      lxLabel->setEnabled(false);
      lxEntry->setEnabled(false);
      lxSet->setEnabled(false);
      lxProgress->setEnabled(false);
      lxProgress->reset();
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
   // progress bar update
   lxProgress->setTotalSteps((int)(lxDelay/0.2));

   lxProgress->reset();
   // lx time update
   lxTimer->stop();
   lxTimer->start((int)(lxDelay*1000));
   // progress timer
   progressTimer->start(200);
   progress=0;
   // text update
   lxEntry->setText(QString().sprintf("%4.2f",lxDelay));
   setProperty("FrameRateSecond",1.0/lxDelay);
   lxControler->startAccumulation();
}

void QCamDC60::lxProgressStep() {
   progress++;
   lxProgress->setProgress(progress);
}

void QCamDC60::lxTimeout() {
   lxControler->stopAccumulation();
   usleep(33000);
   progress=0;
   lxControler->startAccumulation();
}
