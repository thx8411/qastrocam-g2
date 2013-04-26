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

/**********************************/
/* Philips camera support for the */
/*   new pwc driver (kernel 3)    */
/*  driver it still buggy, to be  */
/*     fixed and completed        */
/**********************************/

// only available for kernel 3
#if !KERNEL_2

#include <math.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>

#include <Qt/qradiobutton.h>
#include <Qt/qlcdnumber.h>
#include <Qt/qprogressbar.h>
#include <Qt/qtooltip.h>
#include <Qt/qpushbutton.h>
#include <Qt/qmessagebox.h>
#include <Qt/qlineedit.h>

#include "QCamVestaK3.hpp"
#include "QGridBox.hpp"
#include "QCamSlider.hpp"
#include "QCamRadioBox.hpp"
#include "QCamComboBox.hpp"
#include "SCmod.hpp"
#include "SCmodParPortPPdev.hpp"
#include "QCamUtilities.hpp"

// PWC WHITE BALANCE MODES

#define PWC_WB_AUTO	0
#define PWC_WB_INDOOR	1
#define PWC_WB_OUTDOOR	2
#define PWC_WB_FL	3
#define PWC_WB_MANUAL	4

// PWC CUSTOM COMMANDES

// comes from the PWC driver, hope it won't change...
#define PWC_CID_CUSTOM(ctrl) ((V4L2_CID_USER_BASE | 0xf000) + custom_ ## ctrl)

enum { custom_autocontour, custom_contour, custom_noise_reduction,
        custom_awb_speed, custom_awb_delay,
        custom_save_user, custom_restore_user, custom_restore_factory };

// Vesta class

QCamVesta::QCamVesta(const char * devpath):
   QCamV4L2(devpath,(ioNoBlock|ioUseSelect|haveBrightness|haveContrast|haveColor)) {
   SCmodCtrl_=NULL;
   exposureTimeLeft_=NULL;
   whiteBalanceMode_=-1;

   if (sscanf((char*)v4l2_cap_.card, "Philips %d webcam", &type_) != 1)
      // OEM camera
      type_=0;

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
}



bool QCamVesta::updateFrame() {
   static int tmp;

   cout << "updateFrame" << endl;

   if (skippedFrame_ < multiplicateur_-1) {
      if (skippedFrame_ >= multiplicateur_ -3) {
         stopAccumulation();
      }
      if (QCamV4L2::dropFrame()) {
         skippedFrame_++;
         exposureTimeLeft_->setValue(skippedFrame_);
         tmp=0;
      }
      return false;
   } else {
      if (QCamV4L2::updateFrame()) {
         int tmpVal;

         skippedFrame_=0;
         if (multiplicateur_ > 1) {
            startAccumulation();
            if (guiBuild()) exposureTimeLeft_->reset();
         }

         setProperty("Gain",tmpVal=getGain(),false);
         emit gainChange(tmpVal);

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

   cout << "refreshPictureSettings" << endl;

   QCamV4L2::refreshPictureSettings();
   QCamV4L2::refreshPictureSettings(); // second call needed. if not some value are not properly restored.

   int tmp;
   emit(sharpnessChange(getSharpness()));
   emit(noiseRemovalChange(getNoiseRemoval()));
   emit(gainChange(getGain()));

   //setProperty("CompressionWished",tmp=getCompression());
   //emitcompressionChange(tmp));

   setProperty("Gama",tmp=getGama());
   emit gamaChange(tmp);

   getWhiteBalance();
}

void QCamVesta::saveSettings() {
   struct v4l2_control ctrl;
   ctrl.id=PWC_CID_CUSTOM(save_user);
   if (-1==ioctl(device_,VIDIOC_S_CTRL, &ctrl))
      perror("PWC_CID_CUSTOM(save_user)");
}

void QCamVesta::restoreSettings() {
   struct v4l2_control ctrl;
   ctrl.id=PWC_CID_CUSTOM(restore_user);
   if (-1==ioctl(device_,VIDIOC_S_CTRL, &ctrl))
      perror("PWC_CID_CUSTOM(restore_user)");

   refreshPictureSettings();
   refreshGui_=true;
}

void QCamVesta::restoreFactorySettings() {
   struct v4l2_control ctrl;
   ctrl.id=PWC_CID_CUSTOM(restore_factory);
   if (-1==ioctl(device_,VIDIOC_S_CTRL, &ctrl))
      perror("PWC_CID_CUSTOM(restore_factory)");

   setBestQuality();
   refreshPictureSettings();
   refreshGui_=true;
}

void QCamVesta::setLiveWhiteBalance(bool val) {
   liveWhiteBalance_=val;
   getWhiteBalance();
}

void QCamVesta::setGain(int val) {
   struct v4l2_control ctrl;
   if(val==-1) {
      // auto gain on
      ctrl.id=V4L2_CID_AUTOGAIN;
      ctrl.value=1;
      if (-1 == ioctl(device_,VIDIOC_S_CTRL, &ctrl))
         perror("V4L2_CID_AUTOGAIN on");
   } else {
      // auto gain off
      ctrl.id=V4L2_CID_AUTOGAIN;
      ctrl.value=0;
      if (-1 == ioctl(device_,VIDIOC_S_CTRL, &ctrl))
         perror("V4L2_CID_AUTOGAIN off");
      //  gain setting
      ctrl.id=V4L2_CID_GAIN;
      ctrl.value=val%64;
      if (-1 == ioctl(device_,VIDIOC_S_CTRL, &ctrl)) {
         perror("V4L2_CID_GAIN setting");
      } else
         lastGain_=val;
   }
}

int QCamVesta::getGain() const {
   int gain=0;
   struct v4l2_control ctrl;
   ctrl.id=V4L2_CID_GAIN;
   ctrl.value=0;
   if (-1==ioctl(device_,VIDIOC_G_CTRL, &ctrl)) {
      perror("V4L2_CID_GAIN getting");
      gain=lastGain_;
   } else
      gain=ctrl.value;
      lastGain_=gain;
   return gain;
}

// *************** TO BE FIXED ******************
void QCamVesta::setExposure(int val) {
   struct v4l2_ext_controls ext_ctrls;
   struct v4l2_ext_control ext_ctrl[1];
   struct v4l2_control ctrl;

   //ext_ctrls.ctrl_class=V4L2_CTRL_CLASS_USER;
   ext_ctrls.count=1;
   ext_ctrls.controls=ext_ctrl;
   if(val==-1) {
      // auto exposure on
      ext_ctrl->id=V4L2_CID_EXPOSURE_AUTO;
      ext_ctrl->value=V4L2_EXPOSURE_AUTO;
      //if (-1 == ioctl(device_,VIDIOC_S_EXT_CTRLS, &ext_ctrls))
      //   perror("V4L2_CID_EXPOSURE_AUTO on");
   } else {
      // auto exposure off
      ext_ctrl->id=V4L2_CID_EXPOSURE_AUTO;
      ext_ctrl->value=V4L2_EXPOSURE_MANUAL;
      //if (-1 == ioctl(device_,VIDIOC_S_EXT_CTRLS, &ext_ctrls))
      //   perror("V4L2_CID_EXPSOURE_AUTO off");
      //  exposure setting
      ctrl.id=V4L2_CID_EXPOSURE;
      ctrl.value=val;
      if (-1 == ioctl(device_,VIDIOC_S_CTRL, &ctrl)) {
         perror("V4L2_CID_EXPOSURE setting");
      } else
         setProperty("Exposure",val,false);
   }
}
// ****************************************

int QCamVesta::getExposure() const {
   int exposure=0;
   struct v4l2_control ctrl;
   ctrl.id=V4L2_CID_EXPOSURE;
   ctrl.value=0;
   if (-1==ioctl(device_,VIDIOC_G_CTRL, &ctrl)) {
      perror("V4L2_CID_EXPOSURE getting");
   } else
      exposure=ctrl.value;
   return exposure;
}

// * NOT AVAILABLE IN THE DRIVER ANYMORE *
/*
void QCamVesta::setCompression(int val) {
   //ioctl(device_, VIDIOCPWCSCQUAL, &val);
}
*/
// ****************************************

// * NOT AVAILABLE IN THE DRIVER ANYMORE *
/*int QCamVesta::getCompression() const {
   int compression=0;
   ioctl(device_, VIDIOCPWCGCQUAL , &compression);
   if (compression < 0) compression*=-1;
   return compression;
}
*/
// ****************************************

void QCamVesta::setNoiseRemoval(int val) {
   struct v4l2_control ctrl;
   ctrl.id=PWC_CID_CUSTOM(noise_reduction);
   ctrl.value=val;
   if (-1 == ioctl(device_,VIDIOC_S_CTRL, &ctrl)) {
      perror("PWC_CUSTOM(noise_reduction) setting");
   }
   emit(noiseRemovalChange(getNoiseRemoval()));
}

int QCamVesta::getNoiseRemoval() const {
   int noise=0;
   struct v4l2_control ctrl;
   ctrl.id=PWC_CID_CUSTOM(noise_reduction);
   if (-1 == ioctl(device_,VIDIOC_G_CTRL, &ctrl)) {
      perror("PWC_CUSTOM(noise_reduction) setting");
   } else
      noise=ctrl.value;
   return noise;
}

void QCamVesta::setSharpness(int val) {
struct v4l2_control ctrl;
   if(val==-1) {
      // auto sharpness on
      ctrl.id=PWC_CID_CUSTOM(autocontour);
      ctrl.value=1;
      if (-1 == ioctl(device_,VIDIOC_S_CTRL, &ctrl))
         perror("PWC_CID_CUSTOM(autocontour) on");
   } else {
      // auto sharpness off
      ctrl.id=PWC_CID_CUSTOM(autocontour);
      ctrl.value=0;
      if (-1 == ioctl(device_,VIDIOC_S_CTRL, &ctrl))
         perror("PWC_CID_CUSTOM(autocontour) off");
      //  gain setting
      ctrl.id=PWC_CID_CUSTOM(contour);
      ctrl.value=val%64;
      if (-1 == ioctl(device_,VIDIOC_S_CTRL, &ctrl))
         perror("PWC_CID_CUSTOM(contour) setting");
   }
   emit(sharpnessChange(getSharpness()));
}

int QCamVesta::getSharpness() const {
   int sharp=0;
   struct v4l2_control ctrl;
   ctrl.id=PWC_CID_CUSTOM(contour);
   if (-1 == ioctl(device_,VIDIOC_G_CTRL, &ctrl)) {
      perror("PWC_CID_CUSTOM(contour) setting");
   } else
      sharp=ctrl.value;
   return sharp;
}

void QCamVesta::setBackLight(bool val) {
   struct v4l2_control ctrl;
   ctrl.id=V4L2_CID_BACKLIGHT_COMPENSATION;
   ctrl.value=val;
   if (-1 == ioctl(device_,VIDIOC_S_CTRL, &ctrl)) {
      perror("V4L2_CID_BACKLIGHT_COMPENSATION setting");
   }
}

bool QCamVesta::getBackLight() const {
   struct v4l2_control ctrl;
   ctrl.id=V4L2_CID_BACKLIGHT_COMPENSATION;
   ctrl.value=0;
   if (-1 == ioctl(device_,VIDIOC_G_CTRL, &ctrl)) {
      perror("V4L2_CID_BACKLIGHT_COMPENSATION getting");
   }
   return ctrl.value;
}

void QCamVesta::setFlicker(bool val) {
   struct v4l2_control ctrl;
   ctrl.id=V4L2_CID_BAND_STOP_FILTER;
   ctrl.value=val;
   if (-1 == ioctl(device_,VIDIOC_S_CTRL, &ctrl)) {
      perror("V4L2_CID_BAND_STOP_FILTER setting");
   }
}

bool QCamVesta::getFlicker() const {
   struct v4l2_control ctrl;
   ctrl.id=V4L2_CID_BAND_STOP_FILTER;
   ctrl.value=0;
   if (-1 == ioctl(device_,VIDIOC_G_CTRL, &ctrl)) {
      perror("V4L2_CID_BAND_STOP_FILTER getting");
   }
   return ctrl.value;
}

void QCamVesta::setGama(int val) {
   struct v4l2_control ctrl;

   picture_.whiteness=val;
   ctrl.id=V4L2_CID_WHITENESS;
   ctrl.value=val;
   if (-1 == ioctl(device_,VIDIOC_S_CTRL, &ctrl)) {
      perror("V4L2_CID_WHITENESS setting");
   }
}

int QCamVesta::getGama() const {
   struct v4l2_control ctrl;

   ctrl.id=V4L2_CID_WHITENESS;
   ctrl.value=0;
   if (-1 == ioctl(device_,VIDIOC_G_CTRL, &ctrl)) {
      perror("V4L2_CID_WHITENESS getting");
   }
   return ctrl.value;
}

void QCamVesta::setFrameRate(int value) {
   struct v4l2_streamparm parms;
   struct v4l2_requestbuffers mmap_reqbuf;

   // mmap stuff, if needed
   memset(&mmap_reqbuf,0,sizeof(mmap_reqbuf));
   mmap_reqbuf.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
   mmap_reqbuf.memory=V4L2_MEMORY_MMAP;
   mmap_reqbuf.count=-1;

   if(useMmap) {
      // stopping stream
      if (-1 == ioctl(device_,VIDIOC_STREAMOFF,&mmap_reqbuf.type)) {
         perror("VIDIOC_STREAMOFF");
      }
   }

   // setting new frame rate
   parms.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
   parms.parm.capture.timeperframe.denominator=value;
   parms.parm.capture.timeperframe.numerator=1;
   if (-1 == ioctl(device_,VIDIOC_S_PARM, &parms)) {
      perror("VIDIOC_S_PARM setting");
   } else {
      setProperty("FrameRateSecond",value/(double)multiplicateur_);
      emit exposureTime(multiplicateur_/(double)getFrameRate());
   }

   // restarting stream
   if(useMmap) {
      if (-1 == ioctl(device_,VIDIOC_STREAMON,&mmap_reqbuf.type)) {
         perror("VIDIOC_STREAMON");
      }
   }
}

int QCamVesta::getFrameRate() const {
   int fps=10; // default fps
   struct v4l2_streamparm parms;
   parms.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
   if (-1 == ioctl(device_,VIDIOC_G_PARM, &parms)) {
      // temporary removed, not working yet
      //perror("VIDIOC_G_PARM getting");
   } else
      fps=(int)(parms.parm.capture.timeperframe.denominator/parms.parm.capture.timeperframe.numerator);
   return (fps);
}

// ************* TODO ****************
void QCamVesta::getWhiteBalance() {

   cout << "getWhiteBalance" << endl;

/*   struct pwc_whitebalance tmp_whitebalance;
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
/*      whiteBalanceMode_=tmp_whitebalance.mode;
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
   */
}
// *****************************

// ********* TODO **************
void QCamVesta::setWhiteBalance() {

   cout << "setWhiteBalance" << endl;

   /*struct pwc_whitebalance wb;
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
   */
}
// ******************************

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
      startAccumulation();
   } else if (multiplicateur_ > 1 && val==1) {
      SCmodCtrl_->leaveLongPoseMode();
      if (guiBuild()) exposureTimeLeft_->hide();
      stopAccumulation();
   }
   multiplicateur_=val;
   if (guiBuild()) {
      remoteCTRLexposure_->setDisabled(multiplicateur_>1);
      exposureTimeLeft_->setMinimum(0);
      exposureTimeLeft_->setMaximum(multiplicateur_-1);
   }
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
   if (mult <=1) {
      setFrameRateMultiplicateur(1);
   } else {
      setFrameRateMultiplicateur(mult);
   }
}

void QCamVesta::initRemoteControlLongExposure(QWidget * remoteCTRL) {
   char tmp[32];
   longExposureTime_ = new  QLineEdit(remoteCTRL);
   longExposureTime_->setMaxLength(4);
   sprintf(tmp,"%4.2f",1.0/frameRate_);
   longExposureTime_->setText(tmp);
   longExposureTime_->setEnabled(false);
   connect(longExposureTime_,SIGNAL(textChanged(const QString&)),this,SLOT(setLongExposureTime(const QString&)));
   QToolTip::add(longExposureTime_,tr("exposure time in secondes (0 to disable)"));

   exposureTimeLeft_=new QProgressBar(remoteCTRL);
   exposureTimeLeft_->hide();
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
   QGridBox * sliders= new QGridBox(VctrlBox,Qt::Vertical,3);

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
   }
   remoteCTRLgama_=new QCamSlider(tr("Gamma"),false,sliders,0,31, false, false);
   QToolTip::add(remoteCTRLgama_,tr("Low gamma implies less contrasts"));
   remoteCTRLgain_=new QCamSlider(tr("Gain"),true,sliders,0,63,false,false);
   QToolTip::add(remoteCTRLgain_,tr("More Gain implies more noise in the images"));

   /* TODO : set the exposure limit depending on camera */
   remoteCTRLexposure_=new QCamSlider(tr("Exp."),true,sliders,0,/*65535*/255,false);
   QToolTip::add(remoteCTRLexposure_,
                 tr("More exposure reduce noise in images.\n"
                 "(manual exposure setting don't work on type 740\n"
                 "if automatic gain is activated).")
      );
   if (QCamUtilities::expertMode()) {
      remoteCTRLcompression_=new QCamSlider(tr("Comp."),false,sliders,0,3);
      remoteCTRLnoiseRemoval_=new QCamSlider(tr("Noise"),false,sliders,0,3);
      remoteCTRLsharpness_=new QCamSlider(tr("Sharp."),true,sliders,0,63,false,false);

      connect(this,SIGNAL(compressionChange(int)),remoteCTRLcompression_,SLOT(setValue(int)));
      connect(remoteCTRLcompression_,SIGNAL(valueChange(int)),this,SLOT(setCompression(int)));
      connect(this,SIGNAL(noiseRemovalChange(int)),remoteCTRLnoiseRemoval_,SLOT(setValue(int)));
      connect(remoteCTRLnoiseRemoval_,SIGNAL(valueChange(int)),this,SLOT(setNoiseRemoval(int)));

      QToolTip::add(remoteCTRLnoiseRemoval_,tr("Dynamic Noise removal (0=none, 3=high) (0 give brighter image)"));

      connect(this,SIGNAL(sharpnessChange(int)),remoteCTRLsharpness_,SLOT(setValue(int)));
      connect(remoteCTRLsharpness_,SIGNAL(valueChange(int)),this,SLOT(setSharpness(int)));
      QToolTip::add(remoteCTRLsharpness_,tr("Shaprness enhancement (0=none, 65536=high) (low value blurs image)"));

      //
      // TEMP
      // driver is still incomplete, disabling useless stuff
      //
      remoteCTRLcompression_->setDisabled(true);
      remoteCTRLnoiseRemoval_->setDisabled(true);
      remoteCTRLsharpness_->setDisabled(true);
   }

   int wbValue[]={PWC_WB_AUTO, PWC_WB_INDOOR, PWC_WB_OUTDOOR, PWC_WB_FL, PWC_WB_MANUAL};
   const char *wbLabel[]={"Auto", "In","Out","Neon","Manual"};
   remoteCTRLWhiteBalance_=new QCamRadioBox(tr("White Balance"),VctrlBox,5,wbValue,wbLabel,5);

   connect(remoteCTRLWhiteBalance_,SIGNAL(change(int)),this,SLOT(setWhiteBalanceMode(int)));
   connect(this,SIGNAL(whiteBalanceModeChange(int)),remoteCTRLWhiteBalance_,SLOT(update(int)));

   QWidget* whiteBalanceGroup=new QWidget();
   QHBoxLayout* whiteBalanceGroup_layout= new QHBoxLayout();
   whiteBalanceGroup->setLayout(whiteBalanceGroup_layout);

   QCheckBox* liveWBupdateB = new QCheckBox(tr("live"),whiteBalanceGroup);
   connect(liveWBupdateB,SIGNAL(toggled(bool)),this,SLOT(setLiveWhiteBalance(bool)));
   QToolTip::add(liveWBupdateB,tr("Live Update of red/blue value in automatic mode"));
   whiteBalanceGroup_layout->addWidget(liveWBupdateB);

   remoteCTRLWBred_ = new QCamSlider(tr("red bal."),false,whiteBalanceGroup,0,65535);
   whiteBalanceGroup_layout->addWidget(remoteCTRLWBred_);
   connect(this,SIGNAL(whiteBalanceRedChange(int)),remoteCTRLWBred_,SLOT(setValue(int)));
   connect(remoteCTRLWBred_,SIGNAL(valueChange(int)),this,SLOT(setWhiteBalanceRed(int)));

   remoteCTRLWBblue_ = new QCamSlider(tr("blue bal."),false,whiteBalanceGroup,0,65535);
   whiteBalanceGroup_layout->addWidget(remoteCTRLWBblue_);
   connect(this,SIGNAL(whiteBalanceBlueChange(int)),remoteCTRLWBblue_,SLOT(setValue(int)));
   connect(remoteCTRLWBblue_,SIGNAL(valueChange(int)),this,SLOT(setWhiteBalanceBlue(int)));

   remoteCTRLWhiteBalance_->layout()->addWidget(whiteBalanceGroup);

   remoteCTRLWBred_->setEnabled(false);
   remoteCTRLWBblue_->setEnabled(false);

   exposureTimeLeft_=NULL;
   connect(this,SIGNAL(gainChange(int)),remoteCTRLgain_,SLOT(setValue(int)));
   connect(remoteCTRLgain_,SIGNAL(valueChange(int)),this,SLOT(setGain(int)));

   connect(remoteCTRLexposure_,SIGNAL(valueChange(int)),this,SLOT(setExposure(int)));
   connect(this,SIGNAL(gamaChange(int)),remoteCTRLgama_,SLOT(setValue(int)));
   connect(remoteCTRLgama_,SIGNAL(valueChange(int)),this,SLOT(setGama(int)));

   QCamHBox * settings=new QCamHBox(VctrlBox);
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

   remoteCTRLframeRate_ =new QCamHGroupBox(tr("fps / long exposure"),remoteCTRL);
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

   //
   // TEMP
   // driver is still incomplete, disabling useless stuff
   //
   remoteCTRLWhiteBalance_->setDisabled(true);
   saveSettingsB->setDisabled(true);
   restoreSettingsB->setDisabled(true);
   restoreFactorySettingsB->setDisabled(true);
   remoteCTRLframeRate2_->setDisabled(true);
   SCmodSelector_->setDisabled(true);
   remoteCTRL->show();

   return remoteCTRL;
}

void QCamVesta::setBestQuality() {
   setNoiseRemoval(0);
   setSharpness(0);
   //setCompression(0);
}


// ******* TODO *********
// not available in the driver anymore
void QCamVesta::setLed(int on, int off) const {

   cout << "setLed" << endl;

   if (haveLeds_) {
      //struct pwc_leds leds;
      //leds.led_on=on;
      //leds.led_off=off;
      //if (ioctl(device_, VIDIOCPWCSLED, &leds)) {
      //   haveLeds_=false;
      //}
   }
}
// **********************

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

#endif /* !KERNEL_2 */
