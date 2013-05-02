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

#include <math.h>

#include <Qt/qlabel.h>

#include "SCmodParPortPPdev.hpp"
#include "QCamV4L2lx.hpp"

QCamV4L2lx::QCamV4L2lx(const char * devpath):
   QCamV4L2(devpath,ioNoBlock|ioUseSelect|haveBrightness|haveContrast|haveHue|haveColor) {
   // some lx widgets init to avoid segfaults in updateFrame
   lxBar=NULL;
   // ******************
   // lx mode vars inits
   // ******************
   // lx mod use 0.2s steps :
   // this is the smallest common value for PAL and NTSC
   // PAL : 0.2s = 5 full frames
   // NTSC : 0.2s = 6 full frames
   lxDelay=0.2;
   lxLevel=64;
   lxControler=NULL;
   lxEnabled=false;
   lxBaseTime=getTime();
   // lx timer settings
   // we use a QT timer for long exposure mode timing
   lxTimer=new QTimer(this);
   lxTimer->stop();
   connect(lxTimer,SIGNAL(timeout()),this,SLOT(LXframeReady()));
}

bool QCamV4L2lx::updateFrame() {
   if(lxEnabled) {
      if(lxFramesToDrop>1) {
         // we still drop frames
         lxFrameCounter++;
         // update progress bar
         if(lxBar) lxBar->setValue(lxFrameCounter);
         dropFrame();
         return(true);
      } else if (lxFramesToDrop==1) {
         // last frame to drop
         lxFramesToDrop--;
         lxFrameCounter++;
         // update progress bar
         if(lxBar) lxBar->setValue(lxFrameCounter);
         dropFrame();
         return(true);
      } else {
         // we have a frame, reset for the next period;
         lxFramesToDrop=2;
         // resetting dropped frames counter
         lxFrameCounter=0;
         // resetting progress bar
         if(lxBar) lxBar->reset();
         lxControler->startAccumulation();
      }
   }
   QCamV4L2::updateFrame();

   // update framerate
   if(!lxEnabled)
      lxRate->setText(QString().sprintf("%i",frameRate_));
}

QWidget * QCamV4L2lx::buildGUI(QWidget * parent) {
   QWidget * remoteCTRL=QCamV4L2::buildGUI(parent);

   // V4L generic long exposure
   // container
   remoteCTRLlx= new QCamHGroupBox(tr("long exposure"),remoteCTRL);
   // frame rate display
   lxLabel1= new QLabel("fps :",remoteCTRLlx);
   lxRate= new QLabel(QString().sprintf("%i",frameRate_),remoteCTRLlx);
   lxRate->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lxRate->setMinimumWidth(32);
   // lx mode selector
   int lxTable[]={lxNone,lxPar,lxSer};
   const char* lxLabel[]={"lx : none","lx : // port","lx : serial"};
   lxSelector=new QCamComboBox("lxMode",remoteCTRLlx,3,lxTable,lxLabel);
   // integration delay
   lxLabel2=new QLabel("Delay :",remoteCTRLlx);
   lxTime=new QLineEdit(remoteCTRLlx);
   lxTime->setMaximumWidth(48);
   lxTime->setEnabled(false);
   lxTime->setText(QString().sprintf("%4.2f",1.0/(double)frameRate_));
   // integration delay button
   lxSet=new QPushButton("Set",remoteCTRLlx);
   lxSet->setMaximumWidth(32);
   lxSet->setEnabled(false);
   // progress bar
   lxBar=new QProgressBar(remoteCTRLlx);
   lxBar->setMinimum(0);
   lxBar->setMaximum(1);
   lxBar->reset();
   // tips
   lxRate->setToolTip("Current frame rate");
   lxSelector->setToolTip("Long exposure mode");
   lxTime->setToolTip("Integration time in seconds (0.2s steps)");
   lxSet->setToolTip("Set integration time");
   lxBar->setToolTip("Integration progress bar");
   // default lx delay, init
   lxDelay=0.2;
   lxControler=NULL;
   // lx events connector
   connect(lxSelector,SIGNAL(change(int)),this,SLOT(setLXmode(int)));
   connect(lxSet,SIGNAL(released()),this,SLOT(setLXtime()));

   return(remoteCTRL);
}

// setting lx modes
void QCamV4L2lx::setLXmode(int value) {
   if(lxControler) {
      lxControler->leaveLongPoseMode();
      delete(lxControler);
      lxControler=NULL;
   }
   switch(value) {
      // lx mode disabled
      case lxNone :
         // object allready deleted
         lxRate->setText(QString().sprintf("%i",frameRate_));
         lxTime->setText(QString().sprintf("%4.2f",1.0/(double)frameRate_));
         lxTime->setEnabled(false);
         lxSet->setEnabled(false);
         lxBar->setMinimum(0);
         lxBar->setMaximum(1);
         lxBar->reset();
         setProperty("FrameRateSecond",frameRate_);
         lxEnabled=false;
         lxTimer->stop();
         break;
      // parallel mode lx on
      case lxPar :
         lxRate->setText("N/A");
         lxControler=new SCmodParPortPPdev();
         setLXtime();
         lxTime->setEnabled(true);
         lxSet->setEnabled(true);
         lxBar->reset();
         lxEnabled=true;
         lxTimer->start((int)(lxDelay*1000));
         lxControler->enterLongPoseMode();
         lxFrameCounter=0;
         lxFramesToDrop=2;
         lxControler->startAccumulation();
         break;
      // serial mode lx on
      case lxSer :
         lxRate->setText("N/A");
         lxControler=new SCmodSerialPort();
         setLXtime();
         lxTime->setEnabled(true);
         lxSet->setEnabled(true);
         lxBar->reset();
         lxEnabled=true;
         lxTimer->start((int)(lxDelay*1000));
         lxControler->enterLongPoseMode();
         lxFrameCounter=0;
         lxFramesToDrop=2;
         lxControler->startAccumulation();
         break;
   }
}

// changing integration time
void QCamV4L2lx::setLXtime() {
   float val;
   // reading edit line and converts
   QString str=lxTime->text();
   if (sscanf(str.toLatin1(),"%f",&val)!=1) {
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
   lxBar->setMinimum(0);
   lxBar->setMaximum((int)(lxDelay*frameRate_));
   lxBar->reset();
   // lx time update
   lxTimer->stop();
   lxTimer->start((int)(lxDelay*1000));
   lxFrameCounter=0;
   lxTime->setText(QString().sprintf("%4.2f",lxDelay));
   setProperty("FrameRateSecond",1.0/lxDelay);
   lxControler->startAccumulation();
}

// lx timer timeout slot, stops integration
void QCamV4L2lx::LXframeReady() {
   // stop integration
   lxControler->stopAccumulation();
   // last frame to drop
   lxFramesToDrop=1;
}

