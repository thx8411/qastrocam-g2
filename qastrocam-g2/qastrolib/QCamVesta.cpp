/******************************************************************
Qastrocam
Copyright (C) 2003-2009   Franck Sicard
Qastrocam-g2
Copyright (C) 2009-2010   Blaise-Florentin Collin

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


#include "QCamVesta.moc"
#include <iostream>
#include <math.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <qradiobutton.h>
#include <qvbuttongroup.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qhbuttongroup.h>
#include <qlcdnumber.h>
#include <qprogressbar.h>
#include <qtooltip.h>
#include <qhgroupbox.h>
#include <qpushbutton.h>
#include <qmessagebox.h>

#include "QCamSlider.hpp"
#include "QGridBox.hpp"
#include "QCamRadioBox.hpp"
#include "QCamComboBox.hpp"
#include "qlineedit.h"
#include "SCmod.hpp"
#include "SCmodParPortPPdev.hpp"
#include "QCamUtilities.hpp"

QCamVesta::QCamVesta(const char * devpath):
   QCamV4L2(devpath,(ioNoBlock|ioUseSelect|haveBrightness|haveContrast|haveColor)) {
   SCmodCtrl_=NULL;
   exposureTimeLeft_=NULL;
   whiteBalanceMode_=-1;

   {
      /* sanity check */
      bool IsPhilips = false;
      struct pwc_probe probe;
      if (sscanf((char*)v4l2_cap_.card, "Philips %d webcam", &type_) == 1) {
         /* original phillips */
         IsPhilips = true;
      } else if (ioctl(device_, VIDIOCPWCPROBE, &probe) == 0) {
         /* an OEM clone ? */
         if (!strcmp((char*)v4l2_cap_.card,probe.name)) {
           IsPhilips = true;
           type_=probe.type;
         }
      }
      if (!IsPhilips) {
      QMessageBox::information(0,"Qastrocam-g2","QCamVesta::QCamVesta() called on a non Philips Webcam.\ndid you use QCamV4L2::openBestDevice() to open your device?");
         cout << "QCamVesta::QCamVesta() called on a non Philips Webcam.\n"
              << "did you use QCamV4L2::openBestDevice() to open your device?"
              << endl;
         exit(1);
      }
   }

   //initRemoteControl(remoteCTRL_);
   multiplicateur_=1;
   if (exposureTimeLeft_) exposureTimeLeft_->hide();
   skippedFrame_=0;
   liveWhiteBalance_=false /* set to true to show dynamicaly white balance
                              setting (slow) */;
   refreshGui_=true;
   haveLeds_= (type_ >= 730);
   setLed(0,255);
   setWhiteBalanceMode(PWC_WB_AUTO);
   getWhiteBalance();
   lastGain_=getGain();

   // must be rewritten for V4L2

   // read the window_ values
   // generic V4L don't do it anymore
   //if(ioctl(device_,VIDIOCGWIN, &window_))
      //perror("ioctl (VIDIOCGWIN)");
}



bool QCamVesta::updateFrame() {
   static int tmp;
   if (skippedFrame_ < multiplicateur_-1) {
      if (skippedFrame_ >= multiplicateur_ -3) {
         stopAccumulation();
      }
      if (QCamV4L2::dropFrame()) {
         skippedFrame_++;
         exposureTimeLeft_->setProgress(skippedFrame_);
         tmp=0;
      }
      return false;
   } else {
      if (QCamV4L2::updateFrame()) {
         skippedFrame_=0;
         if (multiplicateur_ > 1) {
            startAccumulation();
            if (guiBuild()) exposureTimeLeft_->reset();
         }
         int tmpVal;
         setProperty("Gain",tmpVal=getGain(),false);
         emit gainChange(tmpVal);
         //emit exposureChange(getExposure());
         //setProperty("Gama",tmpVal=getGama());
         //emit gamaChange(tmpVal);
         //emit compressionChange(getCompression());
         /*
           setProperty("NoiseRemoval",tmpVal=getNoiseRemoval());
           emit noiseRemovalChange(tmpVal);
           setProperty("Sharpness",tmpVal=getSharpness());
           emit sharpnessChange(tmpVal);
         */
         setProperty("FrameRateSecond",(tmpVal=getFrameRate())/(double)multiplicateur_);
         emit frameRateChange(tmpVal);
         if (liveWhiteBalance_ || refreshGui_) {
            getWhiteBalance();
         }
         if (SCmodCtrl_) {
            setProperty("ExposureTime",multiplicateur_/(double)getFrameRate());
            emit frameRateMultiplicateurChange(multiplicateur_);
            emit exposureTime(multiplicateur_/(double)getFrameRate());
         } else {
            setProperty("ExposureTime",1/(double)getFrameRate());
         }
         refreshGui_=false;
         return true;
      } else {
         refreshGui_=false;
         return false;
      }
   }
}

void QCamVesta::refreshPictureSettings() {
   QCamV4L2::refreshPictureSettings();
   QCamV4L2::refreshPictureSettings(); // second call needed. if not some value are not properly restored.

   int tmp;
   emit(sharpnessChange(getSharpness()));
   emit(noiseRemovalChange(getNoiseRemoval()));
   emit(gainChange(getGain()));

   setProperty("CompressionWished",tmp=getCompression());
   emit(compressionChange(tmp));

   setProperty("Gama",tmp=getGama());
   emit gamaChange(tmp);

   getWhiteBalance();
}

void QCamVesta::saveSettings() {
   if (ioctl(device_, VIDIOCPWCSUSER)==-1) {
      perror("VIDIOCPWCSUSER");
   }
}

void QCamVesta::restoreSettings() {
   ioctl(device_, VIDIOCPWCRUSER);
   refreshPictureSettings();
   refreshGui_=true;
}

void QCamVesta::restoreFactorySettings() {
   ioctl(device_, VIDIOCPWCFACTORY);
   setBestQuality();
   refreshPictureSettings();
   refreshGui_=true;
}

void QCamVesta::setLiveWhiteBalance(bool val) {
   liveWhiteBalance_=val;
   getWhiteBalance();
}

void QCamVesta::setGain(int val) {
   if(-1==ioctl(device_, VIDIOCPWCSAGC, &val)) {
      perror("VIDIOCPWCSAGC");
   } else {
      lastGain_=val;
   }
}

int QCamVesta::getGain() const {
   int gain;
   static int cpt=0;
   if ((cpt%4)==0) {
      if (-1==ioctl(device_, VIDIOCPWCGAGC, &gain)) {
         perror("VIDIOCPWCGAGC");
         gain=lastGain_;
      } else {
         ++cpt;
         lastGain_=gain;
      }
   } else {
      ++cpt;
      gain=lastGain_;
   }
   if (gain < 0) gain*=-1;
   return gain;
}

void QCamVesta::setExposure(int val) {
   if (-1==ioctl(device_, VIDIOCPWCSSHUTTER, &val)) {
      perror("VIDIOCPWCSSHUTTER");
   } else {
      setProperty("Exposure",val,false);
   }
}

/*
int QCamVesta::getExposure() const {
   int gain;
   ioctl(device_, VIDIOCPWCGSHUTTER, &gain);
   if (gain < 0) gain*=-1;
   return gain;
}
*/

void QCamVesta::setCompression(int val) {
   ioctl(device_, VIDIOCPWCSCQUAL, &val);
}

int QCamVesta::getCompression() const {
   int gain;
   ioctl(device_, VIDIOCPWCGCQUAL , &gain);
   if (gain < 0) gain*=-1;
   return gain;
}

void QCamVesta::setNoiseRemoval(int val) {
   if (-1 == ioctl(device_, VIDIOCPWCSDYNNOISE, &val)) {
       perror("VIDIOCPWCGDYNNOISE");
   }
   emit(sharpnessChange(getSharpness()));
}

int QCamVesta::getNoiseRemoval() const {
   int gain;
   if (-1 == ioctl(device_, VIDIOCPWCGDYNNOISE , &gain)) {
      perror("VIDIOCPWCGDYNNOISE");
   }
   cout <<"get noise = "<<gain<<endl;
   return gain;
}

void QCamVesta::setSharpness(int val) {
   if (-1 == ioctl(device_, VIDIOCPWCSCONTOUR, &val)) {
       perror("VIDIOCPWCSCONTOUR");
   }
   emit(noiseRemovalChange(getNoiseRemoval()));
}

int QCamVesta::getSharpness() const {
   int gain;
   if (-1 == ioctl(device_, VIDIOCPWCGCONTOUR, &gain)) {
      perror("VIDIOCPWCGCONTOUR");
   }
   cout <<"get sharpness = "<<gain<<endl;
   return gain;
}

void QCamVesta::setBackLight(bool val) {
   static int on=1;
   static int off=0;
   if (-1 == ioctl(device_,VIDIOCPWCSBACKLIGHT, &  val?&on:&off)) {
      perror("VIDIOCPWCSBACKLIGHT");
   }
}

bool QCamVesta::getBackLight() const {
   int val;
   if (-1 == ioctl(device_,VIDIOCPWCGBACKLIGHT, & val)) {
      perror("VIDIOCPWCSBACKLIGHT");
   }
   return val !=0;
}

void QCamVesta::setFlicker(bool val) {
   static int on=1;
   static int off=0;
   if (-1 == ioctl(device_,VIDIOCPWCSFLICKER, val?&on:&off)) {
      perror("VIDIOCPWCSFLICKER");
   }
}

bool QCamVesta::getFlicker() const {
   int val;
   if (-1 == ioctl(device_,VIDIOCPWCGFLICKER, & val)) {
      perror("VIDIOCPWCGFLICKER");
   }
   return val !=0;
}

void QCamVesta::setGama(int val) {
   struct v4l2_control ctrl;
   picture_.whiteness=val;
   ctrl.id=V4L2_CID_WHITENESS;
   ctrl.value=val;
   if (-1 == ioctl(device_,VIDIOC_S_CTRL, &ctrl)) {
      perror("");
   }
   //updatePictureSettings();
}

int QCamVesta::getGama() const {
   struct v4l2_control ctrl;
   ctrl.id=V4L2_CID_WHITENESS;
   ctrl.value=0;
   if (-1 == ioctl(device_,VIDIOC_G_CTRL, &ctrl)) {
      perror("");
   }
   return ctrl.value;
}

void QCamVesta::setFrameRate(int value) {

   // must be rewritten for V4L2


   /*
   int res;
   // update the window_
   if(ioctl(device_,VIDIOCGWIN, &window_))
      perror("ioctl (VIDIOCGWIN)");
   window_.flags = (window_.flags & ~PWC_FPS_MASK) | ((value << PWC_FPS_SHIFT) & PWC_FPS_FRMASK);
   res=ioctl(device_, VIDIOCSWIN, &window_);
   if (res!=0) {
      QMessageBox::information(0,"Qastrocam-g2","Frame rate is to high for low compression");
      // looking for the nearest supported framerate
      while((value!=0)&&(res!=0)) {
         value--;
         window_.flags = (window_.flags & ~PWC_FPS_MASK) | ((value << PWC_FPS_SHIFT) & PWC_FPS_FRMASK);
         res=ioctl(device_, VIDIOCSWIN, &window_);
      }
      remoteCTRLframeRate2_->update(value);
      perror("setFrameRate");
   } else {
      ioctl(device_, VIDIOCGWIN, &window_);
      setProperty("FrameRateSecond",value/(double)multiplicateur_);
      emit exposureTime(multiplicateur_/(double)getFrameRate());
   }
   */
}

int QCamVesta::getFrameRate() const {

   // must be rewritten for V4L2
   return (0/*(window_.flags&PWC_FPS_FRMASK)>>PWC_FPS_SHIFT*/);
}

void QCamVesta::getWhiteBalance() {
   struct pwc_whitebalance tmp_whitebalance;
   tmp_whitebalance.mode
      =tmp_whitebalance.manual_red
      =tmp_whitebalance.manual_blue
      =tmp_whitebalance.read_red
      =tmp_whitebalance.read_blue
      =PWC_WB_AUTO;
   if (ioctl(device_, VIDIOCPWCGAWB, &tmp_whitebalance)) {
      perror("getWhiteBalance");
      //whiteBalanceActivated_=false;
      //remoteCTRLWhiteBalance_->hide();
   } else {
      /* manual_red and manual_blue are garbage :-( */
      whiteBalanceMode_=tmp_whitebalance.mode;
      switch(whiteBalanceMode_) {
         case PWC_WB_INDOOR:
            setProperty("WhiteBalanceMode","Indor");
            break;
         case PWC_WB_OUTDOOR:
            setProperty("WhiteBalanceMode","Outdoor");
            break;
         case PWC_WB_FL:
            setProperty("WhiteBalanceMode","Neon");
            break;
         case PWC_WB_MANUAL:
            setProperty("WhiteBalanceMode","Manual");
            whiteBalanceRed_=tmp_whitebalance.manual_red;
            whiteBalanceBlue_=tmp_whitebalance.manual_blue;
            break;
         case PWC_WB_AUTO:
            setProperty("WhiteBalanceMode","Auto");
            whiteBalanceRed_=tmp_whitebalance.read_red;
            whiteBalanceBlue_=tmp_whitebalance.read_blue;
            break;
         default:
            setProperty("WhiteBalanceMode","???");
      }
      emit whiteBalanceModeChange(whiteBalanceMode_);

      if (whiteBalanceMode_ == PWC_WB_MANUAL) {
         setProperty("WhiteBalanceRed",whiteBalanceRed_);
         emit whiteBalanceRedChange(whiteBalanceRed_);
         setProperty("WhiteBalanceBlue",whiteBalanceBlue_);
         emit whiteBalanceBlueChange(whiteBalanceBlue_);
         if (guiBuild()) {
            remoteCTRLWBred_->setEnabled(true);
            remoteCTRLWBblue_->setEnabled(true);
         }
      } else {
         if (guiBuild()) {
            remoteCTRLWBred_->setEnabled(false);
            remoteCTRLWBblue_->setEnabled(false);
            if(liveWhiteBalance_) {
               emit whiteBalanceRedChange(whiteBalanceRed_);
               emit whiteBalanceBlueChange(whiteBalanceBlue_);
            }
         }
      }
   }
}

void QCamVesta::setWhiteBalance() {
   struct pwc_whitebalance wb;
   memset(&wb,0,sizeof(struct pwc_whitebalance));
   wb.mode=whiteBalanceMode_;
   if (wb.mode == PWC_WB_MANUAL) {
      wb.manual_red=whiteBalanceRed_;
      wb.manual_blue=whiteBalanceBlue_;
   }
   if (ioctl(device_, VIDIOCPWCSAWB, &wb)) {
      perror("setWhiteBalance");
      if (guiBuild()) remoteCTRLWhiteBalance_->hide();
   }
   emit whiteBalanceModeChange(whiteBalanceMode_);
}

void QCamVesta::setWhiteBalanceMode(int val) {
   if (val == whiteBalanceMode_) {
      return;
   }
   if (val != PWC_WB_AUTO) {
      if ( val != PWC_WB_MANUAL) {
         whiteBalanceMode_=val;
         setWhiteBalance();
      }
      whiteBalanceMode_=PWC_WB_AUTO;
      setWhiteBalance();
      getWhiteBalance();
   }

   if (guiBuild()) {
      if (val == PWC_WB_MANUAL
          || ( liveWhiteBalance_ && (val ==PWC_WB_AUTO))) {
         remoteCTRLWBred_->setEnabled(true);
         remoteCTRLWBblue_->setEnabled(true);
      } else {
         remoteCTRLWBred_->setEnabled(false);
         remoteCTRLWBblue_->setEnabled(false);
      }
   }
   whiteBalanceMode_=val;
   setWhiteBalance();
   getWhiteBalance();
}

void QCamVesta::setWhiteBalanceRed(int val) {
   whiteBalanceMode_ = PWC_WB_MANUAL;
   whiteBalanceRed_=val;
   setWhiteBalance();
}

void QCamVesta::setWhiteBalanceBlue(int val) {
   whiteBalanceMode_ = PWC_WB_MANUAL;
   whiteBalanceBlue_=val;
   setWhiteBalance();
}

void QCamVesta::setFrameRateMultiplicateur(int val) {
   if (SCmodCtrl_==NULL) { return;}
   if (multiplicateur_ == 1 && val>1) {
      SCmodCtrl_->enterLongPoseMode();
      if (guiBuild()) exposureTimeLeft_->show();
      //emit(sharpnessChange(0));
      //emit(noiseRemovalChange(2));
      startAccumulation();
   } else if (multiplicateur_ > 1 && val==1) {
      SCmodCtrl_->leaveLongPoseMode();
      if (guiBuild()) exposureTimeLeft_->hide();
      stopAccumulation();
   }
   multiplicateur_=val;
   if (guiBuild()) remoteCTRLexposure_->setDisabled(multiplicateur_>1);
   if (guiBuild()) exposureTimeLeft_->setTotalSteps(multiplicateur_-1);
   emit exposureTime(multiplicateur_/(double)getFrameRate());
}


QCamVesta::~QCamVesta() {
   delete SCmodCtrl_;
}

void QCamVesta::setSCmod(int value) {
   if (SCmodCtrl_) {
      setFrameRateMultiplicateur(1);
      delete SCmodCtrl_;
      SCmodCtrl_=NULL;
   }

   switch(value) {
     case SCmodNone:
        // already cleared
        longExposureTime_->setEnabled(false);
        break;
     case SCmodLed:
        setSCmodImpl(new SCmodTucLed(*this));
        longExposureTime_->setEnabled(true);
        setLongExposureTime(longExposureTime_->text());
        break;
     case SCmodSerial:{
        SCmodSerialPort * modSerial=new SCmodSerialPort();
        setSCmodImpl(modSerial);
        longExposureTime_->setEnabled(true);
        setLongExposureTime(longExposureTime_->text());
        }
        break;
     case SCmodPPort2:{
        SCmodParPortPPdev * scMod=new SCmodParPortPPdev();
        setSCmodImpl(scMod);
        longExposureTime_->setEnabled(true);
        setLongExposureTime(longExposureTime_->text());
        }
        break;
   }

   if (SCmodCtrl_ && guiBuild()) {
      SCmodCtrl_->buildGUI(remoteCTRLframeRate_);
   }
}

void QCamVesta::setLongExposureTime(const QString& str) {
   float val;
   if (sscanf(str.latin1(),"%f",&val)!=1) {
      val=0.0;
   }
   int mult=(int)round(val*getFrameRate());
   //float realTime=(float)mult/getFrameRate();
   if (mult <=1) {
      setFrameRateMultiplicateur(1);
   } else {
      setFrameRateMultiplicateur(mult);
   }
   //longExposureTime_->setText(QString().sprintf("%1f",realTime));
}

void QCamVesta::initRemoteControlLongExposure(QWidget * remoteCTRL) {
   //QHGroupBox *  remoteCTRLframeRateGroup
   //   = new QHGroupBox(tr("Long Exposure"),
   //                    remoteCTRL);
   char tmp[32];
   longExposureTime_ = new  QLineEdit(remoteCTRL);
   longExposureTime_->setMaxLength(4);
   sprintf(tmp,"%4.2f",1.0/frameRate_);
   longExposureTime_->setText(tmp);
   longExposureTime_->setEnabled(false);
   connect(longExposureTime_,SIGNAL(textChanged(const QString&)),this,SLOT(setLongExposureTime(const QString&)));
   QToolTip::add(longExposureTime_,tr("exposure time in secondes (0 to disable)"));
   /*
     connect(remoteCTRLframeRateMultiplicateur_,SIGNAL(change(int)),
     this,SLOT(setFrameRateMultiplicateur(int)));
     connect(this,SIGNAL(frameRateMultiplicateurChange(int)),
     remoteCTRLframeRateMultiplicateur_,SLOT(update(int)));
   */
   exposureTimeLeft_=new QProgressBar(remoteCTRL);
   exposureTimeLeft_->hide();
   exposureTimeLeft_->setCenterIndicator(true);
   QToolTip::add(exposureTimeLeft_,tr("Integration progress"));

   exposureTime_=new QLCDNumber(5,remoteCTRL);
   connect(this,SIGNAL(exposureTime(double)),exposureTime_,SLOT(display(double)));
   exposureTime_->setSmallDecimalPoint(false);
   exposureTime_->setSegmentStyle(QLCDNumber::Flat);
   exposureTime_->show();
   QToolTip::add(exposureTime_,tr("Integration time"));

   if (SCmodCtrl_) {
      SCmodCtrl_->buildGUI(remoteCTRL);
   }
}

QWidget *  QCamVesta::buildGUI(QWidget * parent) {
   QWidget * remoteCTRL=QCamV4L2::buildGUI(parent);

   //QHGroupBox* vestaCtrl=new QHGroupBox("Vesta controls",remoteCTRL);
   QGridBox * sliders= new QGridBox(/*vestaCtrl*/VctrlBox,Qt::Vertical,3);
   /*
   QHBox * sliders=new QHBox(remoteCTRL);
   QVBox * left = new QVBox(sliders);
   QVBox * right = new QVBox(sliders);
   */
   if (QCamUtilities::expertMode()) {
      QCheckBox * backLight = new QCheckBox(tr("Back Light"),sliders);
      connect(backLight,SIGNAL(toggled(bool)),this,SLOT(setBackLight(bool)));
      QToolTip::add(backLight,
                    tr("In case the object you are viewing with the camera is\n"
                       "in front of a bright background (for example, a window\n"
                       "or some bright lights), the automatic exposure unit may\n"
                       "make the image too dark. In that case you can turn the\n"
                       "backlight compensation mode off"));
      QCheckBox * flicker = new QCheckBox(tr("Anti-flicker"),sliders);
      connect(flicker,SIGNAL(toggled(bool)),this,SLOT(setFlicker(bool)));
      QToolTip::add(flicker,tr("Suppress 'flickering' of the image when light with a fluo tube"));
      //QCheckBox * rawBayer = new QCheckBox(tr("Raw Bayer"),sliders);
      //connect(rawBayer,SIGNAL(toggled(bool)),this,SLOT(activateRawBayerMode(bool)));
      //QToolTip::add(rawBayer,tr("switch off camera Bayer Matrix => RGB"));
   }
   remoteCTRLgama_=new QCamSlider(tr("Gamma"),false,sliders);
   // not very clean...but works
   remoteCTRLgama_->setMinValue(0);
   remoteCTRLgama_->setMaxValue(32);
   QToolTip::add(remoteCTRLgama_,tr("Low gamma implies less contrasts"));
   remoteCTRLgain_=new QCamSlider(tr("Gain"),true,sliders);
   QToolTip::add(remoteCTRLgain_,tr("More Gain implies more noise in the images"));
   remoteCTRLexposure_=new QCamSlider(tr("Exp."),true,sliders,0,65535,true);
   QToolTip::add(remoteCTRLexposure_,
                 tr("More exposure reduce noise in images.\n"
                 "(manual exposure setting don't work on type 740\n"
                 "if automatic gain is activated).")
      );
   if (QCamUtilities::expertMode()) {
      remoteCTRLcompression_=new QCamSlider(tr("Comp."),false,sliders,0,3);
      remoteCTRLnoiseRemoval_=new QCamSlider(tr("Noise"),false,sliders,0,3);
      remoteCTRLsharpness_=new QCamSlider(tr("Sharp."),false,sliders,0,65535);

      connect(this,SIGNAL(compressionChange(int)),remoteCTRLcompression_,SLOT(setValue(int)));
      connect(remoteCTRLcompression_,SIGNAL(valueChange(int)),this,SLOT(setCompression(int)));
      connect(this,SIGNAL(noiseRemovalChange(int)),remoteCTRLnoiseRemoval_,SLOT(setValue(int)));
      connect(remoteCTRLnoiseRemoval_,SIGNAL(valueChange(int)),this,SLOT(setNoiseRemoval(int)));

      QToolTip::add(remoteCTRLnoiseRemoval_,tr("Dynamic Noise removal (0=none, 3=high) (0 give brighter image)"));

      connect(this,SIGNAL(sharpnessChange(int)),remoteCTRLsharpness_,SLOT(setValue(int)));
      connect(remoteCTRLsharpness_,SIGNAL(valueChange(int)),this,SLOT(setSharpness(int)));
      QToolTip::add(remoteCTRLsharpness_,tr("Shaprness enhancement (0=none, 65536=high) (low value blurs image)"));
   }

   int wbValue[]={PWC_WB_AUTO, PWC_WB_INDOOR, PWC_WB_OUTDOOR, PWC_WB_FL, PWC_WB_MANUAL};
   const char *wbLabel[]={"Auto", "In","Out","Neon","Manual"};
   remoteCTRLWhiteBalance_=new QCamRadioBox(tr("White Balance"),VctrlBox,5,wbValue,wbLabel,5);

   connect(remoteCTRLWhiteBalance_,SIGNAL(change(int)),this,SLOT(setWhiteBalanceMode(int)));
   connect(this,SIGNAL(whiteBalanceModeChange(int)),remoteCTRLWhiteBalance_,SLOT(update(int)));

   QHBox * whiteBalanceSliders = new QHBox(remoteCTRLWhiteBalance_);
   QCheckBox * liveWBupdateB = new QCheckBox(tr("live"),whiteBalanceSliders);

   connect(liveWBupdateB,SIGNAL(toggled(bool)),this,SLOT(setLiveWhiteBalance(bool)));

   QToolTip::add(liveWBupdateB,tr("Live Update of red/blue value in automatic mode"));

   remoteCTRLWBred_ = new QCamSlider(tr("red bal."),false,whiteBalanceSliders,0,65535);

   connect(this,SIGNAL(whiteBalanceRedChange(int)),remoteCTRLWBred_,SLOT(setValue(int)));
   connect(remoteCTRLWBred_,SIGNAL(valueChange(int)),this,SLOT(setWhiteBalanceRed(int)));

   remoteCTRLWBblue_ = new QCamSlider(tr("blue bal."),false,whiteBalanceSliders,0,65535);

   connect(this,SIGNAL(whiteBalanceBlueChange(int)),remoteCTRLWBblue_,SLOT(setValue(int)));
   connect(remoteCTRLWBblue_,SIGNAL(valueChange(int)),this,SLOT(setWhiteBalanceBlue(int)));

   remoteCTRLWBred_->setEnabled(false);
   remoteCTRLWBblue_->setEnabled(false);
   remoteCTRLWhiteBalance_->show();

   exposureTimeLeft_=NULL;
   connect(this,SIGNAL(gainChange(int)),remoteCTRLgain_,SLOT(setValue(int)));
   connect(remoteCTRLgain_,SIGNAL(valueChange(int)),this,SLOT(setGain(int)));

   /*
     connect(this,SIGNAL(exposureChange(int)),remoteCTRLexposure_,
     SLOT(setValue(int)));
   */
   connect(remoteCTRLexposure_,SIGNAL(valueChange(int)),this,SLOT(setExposure(int)));
   connect(this,SIGNAL(gamaChange(int)),remoteCTRLgama_,SLOT(setValue(int)));
   connect(remoteCTRLgama_,SIGNAL(valueChange(int)),this,SLOT(setGama(int)));

   QHBox * settings=new QHBox(VctrlBox);
   QToolTip::add(settings,tr("save/restore settings of gain,exposure & white balance"));

   QPushButton *saveSettingsB =new QPushButton(tr("save"),settings);
   QToolTip::add(saveSettingsB,tr("Save User settings (gain,exposure & white balance)"));
   connect(saveSettingsB,SIGNAL(released()),this,SLOT(saveSettings()));

   QPushButton *restoreSettingsB =new QPushButton(tr("restore"),settings);
   QToolTip::add(restoreSettingsB,tr("Restore User settings"));
   connect(restoreSettingsB,SIGNAL(released()),this,SLOT(restoreSettings()));

   QPushButton *restoreFactorySettingsB =new QPushButton(tr("factory"),settings);
   QToolTip::add(restoreFactorySettingsB,tr("Restore factory default settings"));
   connect(restoreFactorySettingsB,SIGNAL(released()),this,SLOT(restoreFactorySettings()));

   remoteCTRLframeRate_ =new QHGroupBox(tr("fps / long exposure"),remoteCTRL);
   int frameRate[]={5,10,15,20,25,30};
   remoteCTRLframeRate2_=new QCamComboBox(tr("fps"),remoteCTRLframeRate_,6,frameRate,NULL);
   QToolTip::add(remoteCTRLframeRate2_,tr("Camera frame rate"));
   connect(this,SIGNAL(frameRateChange(int)),remoteCTRLframeRate2_,SLOT(update(int)));
   connect(remoteCTRLframeRate2_,SIGNAL(change(int)),this,SLOT(setFrameRate(int)));
   remoteCTRLframeRate2_->show();

   int scModeTable[]={SCmodNone,SCmodPPort2,SCmodLed,SCmodSerial};
   const char* scModeLabel[]={"SC mod : None","SC mod : // port","SC mod : TUC led","SC mod : serial"};
   SCmodSelector_ = new QCamComboBox(tr("SC mod"),remoteCTRLframeRate_,4,scModeTable,scModeLabel);
   QToolTip::add(SCmodSelector_,tr("Long exposure device"));
   connect(SCmodSelector_,SIGNAL(change(int)),this,SLOT(setSCmod(int)));

   initRemoteControlLongExposure(remoteCTRLframeRate_);

   setBestQuality();
   refreshPictureSettings();

   remoteCTRL->show();

   return remoteCTRL;
}

void QCamVesta::setBestQuality() {
   setNoiseRemoval(0);
   setSharpness(0);
   setCompression(0);
}

void QCamVesta::setLed(int on, int off) const {
   if (haveLeds_) {
      struct pwc_leds leds;
      leds.led_on=on;
      leds.led_off=off;
      if (ioctl(device_, VIDIOCPWCSLED, &leds)) {
         haveLeds_=false;
      }
   }
}

void QCamVesta::stopAccumulation() {
   if (SCmodCtrl_) {
      SCmodCtrl_->stopAccumulation();
   }
}

void QCamVesta::startAccumulation() {
   if (SCmodCtrl_) {
      SCmodCtrl_->startAccumulation();
   }
}

const QSize * QCamVesta::getAllowedSize() const {
   if (sizeTable_==NULL) {
      sizeTable_=new QSize[7];
      int currentIndex=0;

      if (getType()<700) sizeTable_[currentIndex++]=QSize(128,96);
      sizeTable_[currentIndex++]=QSize(160,120);
      if (getType()<700) sizeTable_[currentIndex++]=QSize(176,144);
      sizeTable_[currentIndex++]=QSize(320,240);
      if (getType()<700) sizeTable_[currentIndex++]=QSize(352,288);
      sizeTable_[currentIndex++]=QSize(640,480);
      sizeTable_[currentIndex++]=QSize(0,0);
   }
   return sizeTable_;
}

uchar* QCamVesta::mmapCapture() {
   static v4l2_buffer buffer;
   buffer.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
   buffer.memory=V4L2_MEMORY_MMAP;
   // enqueue previous buffer
   //if(ioctl(device_, VIDIOC_QBUF,&buffer)!=0)
   // dequeue new buffer
   if(ioctl(device_, VIDIOC_DQBUF,&buffer)!=0)
      perror("DQBUF");

   return((uchar*)buffers[buffer.index].start);
}
