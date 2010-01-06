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


#include "QCamV4L.moc"
#include <iostream>
#include <sstream>

#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <sys/mman.h>
#include <string>
#include <qtabwidget.h>
#include <qsocketnotifier.h>
#include <qtimer.h>
#include <qtooltip.h>

#include "QCamSlider.hpp"
#include "ccvt.h"
#include "QGridBox.hpp"
#include "QCamComboBox.hpp"
#include "SettingsBackup.hpp"

#include "SCmodParPortPPdev.hpp"

// settings object, needed everywhere
extern settingsBackup settings;

// static value init
const int QCamV4L::DefaultOptions=(haveBrightness|haveContrast|haveHue|haveColor|haveWhiteness);

// constructor
QCamV4L::QCamV4L(const char * devpath,int preferedPalette, const char* devsource,
                 unsigned long options /* cf QCamV4L::options */) {
   // V4L2 needed vars
   v4l2_std_id _id; // video stream standard id
   v4l2_standard standard; // video stream standard
   int _index=0;
   // init defaults value
   options_=options;
   tmpBuffer_=NULL;
   remoteCTRLbrightness_=NULL;
   remoteCTRLcontrast_=NULL;
   remoteCTRLhue_=NULL;
   remoteCTRLcolor_=NULL;
   remoteCTRLwhiteness_=NULL;
   sizeTable_=NULL;
   device_=-1;
   devpath_=devpath;

   // ************************************
   // opening the video device (non block)
   // ************************************
   if (-1 == (device_=open(devpath_.c_str(),O_RDONLY | ((options_ & ioNoBlock)?O_NONBLOCK:0)))) {
      perror(devpath);
   }
   // ************************
   // read device informations
   // v4l2 query cap
   // ************************
   // v4l2
   if (-1 == ioctl(device_,VIDIOC_QUERYCAP,&v4l2_cap_)) {
      perror ("ioctl (VIDIOC_QUERYCAP)");
   }
   cout << "device name : " << v4l2_cap_.card << endl;
   useMmap=((v4l2_cap_.capabilities&V4L2_CAP_STREAMING)!=0);
   // ***********************
   // setting image controls
   // ***********************
   // get bounds
   v4l2_queryctrl qctrl;
   // brightness
   qctrl.id=V4L2_CID_BRIGHTNESS;
   // v4l2
   if(ioctl(device_,VIDIOC_QUERYCTRL,&qctrl)==0) {
      picture_.brightness_min=qctrl.minimum;
      picture_.brightness_max=qctrl.maximum;
   } else
      options_&=~haveBrightness;
   // hue
   qctrl.id=V4L2_CID_HUE;
   // v4l2
   if(ioctl(device_,VIDIOC_QUERYCTRL,&qctrl)==0) {
      picture_.hue_min=qctrl.minimum;
      picture_.hue_max=qctrl.maximum;
   } else
      options_&=~haveHue;
   // saturation
   qctrl.id=V4L2_CID_SATURATION;
   // v4l2
   if(ioctl(device_,VIDIOC_QUERYCTRL,&qctrl)==0) {
      picture_.colour_min=qctrl.minimum;
      picture_.colour_max=qctrl.maximum;
   } else
      options_&=~haveColor;
   // contrast
   qctrl.id=V4L2_CID_CONTRAST;
   // v4l2
   if(ioctl(device_,VIDIOC_QUERYCTRL,&qctrl)==0) {
      picture_.contrast_min=qctrl.minimum;
      picture_.contrast_max=qctrl.maximum;
   } else
      options_&=~haveContrast;
   // whiteness
   qctrl.id=V4L2_CID_WHITENESS;
   // v4l2
   if(ioctl(device_,VIDIOC_QUERYCTRL,&qctrl)==0) {
      picture_.whiteness_min=qctrl.minimum;
      picture_.whiteness_max=qctrl.maximum;
   } else
      options_&=~haveWhiteness;

   // get values
   v4l2_control ctrl;
   // brightness
   ctrl.id=V4L2_CID_BRIGHTNESS;
   // v4l2
   ioctl(device_,VIDIOC_G_CTRL,&ctrl);
   picture_.brightness=ctrl.value;
   // hue
   ctrl.id=V4L2_CID_HUE;
   // v4l2
   ioctl(device_,VIDIOC_G_CTRL,&ctrl);
   picture_.hue=ctrl.value;
   // saturation
   ctrl.id=V4L2_CID_SATURATION;
   // v4l2
   ioctl(device_,VIDIOC_G_CTRL,&ctrl);
   picture_.colour=ctrl.value;
   // contrast
   ctrl.id=V4L2_CID_CONTRAST;
   // v4l2
   ioctl(device_,VIDIOC_G_CTRL,&ctrl);
   picture_.contrast=ctrl.value;
   // whiteness
   ctrl.id=V4L2_CID_WHITENESS;
   // v4l2
   ioctl(device_,VIDIOC_G_CTRL,&ctrl);
   picture_.whiteness=ctrl.value;
   // **************************
   // enumerate available inputs
   // **************************
   cout << endl << "available inputs : " << endl;
   input.index=0;
   // v4l2
   while(!ioctl(device_,VIDIOC_ENUMINPUT,&input)) {
      cout << "input #" << input.index << " : " << input.name << endl;
      input.index++;
   }
   // choosing stored source if there is one
   // the storing key is "SOURCE"+<device_name>
   // so, we can store a prefered source for each diffrent device
   int res;
   string keyName("SOURCE_");
   keyName+=(char*)v4l2_cap_.card;
   // if we allready have a source in params, store it
   if(strlen(devsource)) settings.setKey(keyName.c_str(),devsource);
   // looking for the source in settings file
   if(settings.haveKey(keyName.c_str())) {
      // we have a stored prefered source
      cout << "found stored source : " << settings.getKey(keyName.c_str()) << endl ;
      string source=settings.getKey(keyName.c_str());
      // iterate to find the source
      // in the V4L device
      input.index=0;
      do {
         // v4l2
         res=ioctl(device_,VIDIOC_ENUMINPUT,&input);
         input.index++;
      } while((res==0)&&(strcasecmp(source.c_str(),(char*)input.name)!=0));
      if(res==-1)
        // source not found in V4L device, using default
	cout << "source '" << settings.getKey(keyName.c_str()) << "' not found, using default" << endl;
      else {
         // setting the source in V4L device
         _index=input.index-1;
         // v4l2
         ioctl(device_, VIDIOC_S_INPUT, &_index);
      }
   // no source found, using default
   } else cout << "\nIn order to set the default source\nfor this device, use the -i option\n(generic V4L devices only)\n\n" ;
   // get the used source
   // v4l2
   ioctl(device_,VIDIOC_G_INPUT,&_index);
   deviceSource=_index;
   input.index=_index;
   // v4l2
   ioctl(device_,VIDIOC_ENUMINPUT,&input);
   cout << "using : " << input.name << endl << endl;
   // storing used source
   settings.setKey(keyName.c_str(),(char*)input.name);
   // ********************************************
   // getting video standard to compute frame rate
   // ********************************************
   // v4l2
   if(ioctl(device_,VIDIOC_G_STD,&_id)==-1) {
      perror("Getting Standard");
   }
   // iterate to find the used video standard
   standard.index=0;
   res=0;
   while((res!=-1)&&(standard.id!=_id)) {
      // v4l2
      res=ioctl(device_,VIDIOC_ENUMSTD,&standard);
      standard.index++;
   }
   if(res!=0) {
      // standard unknown, default framerate
      frameRate_=0;
      cout << "unable to get video standard" << endl;
   } else {
      // standard found, computing framerate
      cout << "Video standard : " << standard.name << endl;
      if(standard.frameperiod.numerator==0) {
         frameRate_=0;
      } else {
         frameRate_=standard.frameperiod.denominator/standard.frameperiod.numerator;
      }
   }
   if(frameRate_==0) {
      cout << "unable to get video frame rate" << endl;
      // try to get framerate for pwc
      struct video_window window_;
      // v4l
      if(ioctl(device_,VIDIOCGWIN, &window_)==0)
         frameRate_=(window_.flags&/*PWC_FPS_FRMASK*/0x00FF0000)>>/*PWC_FPS_SHIFT*/16;
      else {
         frameRate_=10;
         cout <<  "Using default Framerate: " << frameRate_ << " fps" << endl;
      }
      cout << endl;
   } else
      cout <<  "Using Framerate : " << frameRate_ << " fps" << endl << endl;
   // *************************
   // palette detection/setting
   // *************************
   init(preferedPalette);
   // *****************************
   // getting cropping capabilities
   // *****************************
   memset(&v4l2_crop_,0,sizeof(v4l2_cropcap));
   v4l2_crop_.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
   // reading crop caps
   if(ioctl(device_,VIDIOC_CROPCAP,&v4l2_crop_)==-1) {
      options_&=~supportCropping;
      // not v4l2...
      cout << "trouble getting device cropping capabilities" << endl;
   } else {
      // testing cropping with default rect.
      struct v4l2_crop cropBounds;
      memset(&cropBounds,0,sizeof(struct v4l2_crop));
      cropBounds.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
      cropBounds.c=v4l2_crop_.defrect;
      if(ioctl(device_,VIDIOC_S_CROP,&cropBounds)==0)
         options_|=supportCropping;
      else {
         options_&=~supportCropping;
         cout << "cropping not supported" << endl;
      }
   }

   // temp
   // get frames intervals


   // *********
   // mmap init
   // *********
   // setting struct
   memset(&mmap_reqbuf,0,sizeof(mmap_reqbuf));
   mmap_reqbuf.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
   mmap_reqbuf.memory=V4L2_MEMORY_MMAP;
   mmap_reqbuf.count=-1;
   // **************
   // size detection
   // **************
   sizeTable=getAllowedSize();
   maxWidth=sizeTable[0].width();
   maxHeight=sizeTable[0].height();
   resize(sizeTable[0]);

   // some lx widgets init to avoid segfaults in updateFrame
   lxBar=NULL;
   lxBlink=NULL;
   // setting up the notifier
   notifier_=NULL;
   // *************************************************
   // notifier (all V4L2 devices must support "select")
   // *************************************************
   notifier_ = new QSocketNotifier(device_, QSocketNotifier::Read, this);
   connect(notifier_,SIGNAL(activated(int)),this,SLOT(updateFrame()));
   cout << "Using select to wait for new frames.\n" << endl;

   // update video stream properties
   setProperty("CameraName",(char*)v4l2_cap_.card);
   setProperty("FrameRateSecond",frameRate_);
   label((char*)v4l2_cap_.card);
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

// resize the stream
void QCamV4L::resize(const QSize & s) {
   setSize(s.width(),s.height());
}

void QCamV4L::init(int preferedPalette) {
   // most palettes use color
   mode_=YuvFrame;
   // getting default values
   v4l2_fmt_.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
   // v4l2
   if(-1 == ioctl(device_,VIDIOC_G_FMT,&v4l2_fmt_))
         perror("ioctl (VIDIOC_G_FMT)");
   // setting prefered palette if we have one
   // also used for forced palette (-p option)
   if (preferedPalette) {
      v4l2_fmt_.fmt.pix.pixelformat=preferedPalette;
      // v4l2
      if (0 == ioctl(device_, VIDIOC_S_FMT, &v4l2_fmt_)) {
         palette="prefered";
         cout << "found preferedPalette " << endl<< endl;
         if(v4l2_fmt_.fmt.pix.pixelformat==V4L2_PIX_FMT_GREY)
            mode_=GreyFrame;
         return;
      }
      cout << "preferedPalette " << " invalid, trying to find one."<< endl;
   }
   // else finding a valid palette
   // in high to low quality order
   /* trying VIDEO_PALETTE_RGB24 */
   v4l2_fmt_.fmt.pix.pixelformat=V4L2_PIX_FMT_RGB24;
   // v4l2
   if ( 0== ioctl(device_, VIDIOC_S_FMT, &v4l2_fmt_)&&(v4l2_fmt_.fmt.pix.pixelformat==V4L2_PIX_FMT_RGB24)) {
      palette="rgb24";
      cout << "found palette RGB24" <<endl<<endl;
      return;
   }
   cout <<"palette RGB24 not supported.\n";
   /* trying VIDEO_PALETTE_YUYV */
   v4l2_fmt_.fmt.pix.pixelformat=V4L2_PIX_FMT_YUYV;
   // v4l2
   if ( 0== ioctl(device_, VIDIOC_S_FMT, &v4l2_fmt_)&&(v4l2_fmt_.fmt.pix.pixelformat==V4L2_PIX_FMT_YUYV)) {
     palette="yuyv";
     cout << "found palette YUYV"<<endl<<endl;
     return;
   }
   cout <<"palette YUYV not supported.\n";
   /* trying VIDEO_PALETTE_YUV420P (Planar) */
   /* yuv420i no more supported */
   v4l2_fmt_.fmt.pix.pixelformat=V4L2_PIX_FMT_YUV420;
   // v4l2
   if (0 == ioctl(device_, VIDIOC_S_FMT, &v4l2_fmt_)&&(v4l2_fmt_.fmt.pix.pixelformat==V4L2_PIX_FMT_YUV420)) {
      palette="yuv420";
      cout << "found palette YUV420"<<endl<<endl;
      return;
   }
   cout <<"palette YUV420P not supported.\n";
   /* trying VIDEO_PALETTE_GREY */
   v4l2_fmt_.fmt.pix.pixelformat=V4L2_PIX_FMT_GREY;
   // v4l2
   if ( 0== ioctl(device_, VIDIOC_S_FMT, &v4l2_fmt_)&&(v4l2_fmt_.fmt.pix.pixelformat==V4L2_PIX_FMT_GREY)) {
      palette="grey";
      cout << "found palette GREY"<<endl<<endl;
      mode_=GreyFrame;
      return;
   }
   cout << "palette GREY not supported.\n";
   // no supported palette, leaving
   cerr <<"could not find a supported palette.\n";
   exit(1);
}

// allocate memory for buffers, depending on
// frame size and palette
void QCamV4L::allocBuffers() {
   delete tmpBuffer_;
   inputBuffer_.setSize(QSize(v4l2_fmt_.fmt.pix.width,v4l2_fmt_.fmt.pix.height));
   switch (v4l2_fmt_.fmt.pix.pixelformat) {
   case V4L2_PIX_FMT_GREY:
      yuvFrameMemSize=v4l2_fmt_.fmt.pix.width * v4l2_fmt_.fmt.pix.height;
      break;
   case V4L2_PIX_FMT_RGB24:
      yuvFrameMemSize=v4l2_fmt_.fmt.pix.width * v4l2_fmt_.fmt.pix.height * 3;
      break;
   case V4L2_PIX_FMT_YUV420:
      yuvFrameMemSize=v4l2_fmt_.fmt.pix.width * v4l2_fmt_.fmt.pix.height * 3/2;
      break;
   case V4L2_PIX_FMT_YUYV:
      yuvFrameMemSize=v4l2_fmt_.fmt.pix.width * v4l2_fmt_.fmt.pix.height * 2;
      break;
   default:
      yuvFrameMemSize=0;
      tmpBuffer_=NULL;
   }
   tmpBuffer_=new uchar[yuvFrameMemSize];
}

// get frame sizes supported by the
// video device
const QSize * QCamV4L::getAllowedSize() const {
   if (sizeTable_==NULL) {
      int currentIndex=0;
      int last_x=0;
      int last_y=0;
      int min_x;
      int min_y;
      sizeTable_=new QSize[8];
      v4l2_format v4l2_fmt_temp;

      cout << "Frame size detection" << endl;

      v4l2_fmt_temp.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
      // get previous values
      // v4l2
      if(-1 == ioctl(device_,VIDIOC_G_FMT,&v4l2_fmt_temp))
         perror("ioctl (VIDIOC_G_FMT)");
      // trying small size to get min size
      v4l2_fmt_temp.fmt.pix.width=1;
      v4l2_fmt_temp.fmt.pix.height=1;
      // v4l2
      if (-1 == ioctl(device_,VIDIOC_TRY_FMT,&v4l2_fmt_temp))
         // VIDIOC_TRY_FMT not supported
         // v4l2
         if (-1 == ioctl(device_,VIDIOC_S_FMT,&v4l2_fmt_temp))
            perror ("ioctl (VIDIOC_S_FMT)");
      min_x=v4l2_fmt_temp.fmt.pix.width;
      min_y=v4l2_fmt_temp.fmt.pix.height;
      // trying huge size to get max size
      v4l2_fmt_temp.fmt.pix.width=INT_MAX;
      v4l2_fmt_temp.fmt.pix.height=INT_MAX;
      // v4l2
      if (-1 == ioctl(device_,VIDIOC_TRY_FMT,&v4l2_fmt_temp))
         // VIDIOC_TRY_FMT not supported
         // v4l2
         if (-1 == ioctl(device_,VIDIOC_S_FMT,&v4l2_fmt_temp))
            perror ("ioctl (VIDIOC_S_FMT)");
      // most of time, v4l generic supports continous 4 or 8 multiple pixel sizes
      // it gives to much diffrent sizes. We test from max size to min size half by
      // half.
      while((currentIndex<7)&&(v4l2_fmt_temp.fmt.pix.width>=min_x)&&(v4l2_fmt_temp.fmt.pix.height>=min_y)) {
         // try the new size...
         // v4l2
         if (ioctl(device_, VIDIOC_TRY_FMT, &v4l2_fmt_temp)!=-1) {
            // ... and store it if it as changed
            if((last_x!=v4l2_fmt_temp.fmt.pix.width)||(last_y!=v4l2_fmt_temp.fmt.pix.height)) {
               sizeTable_[currentIndex]=QSize(v4l2_fmt_temp.fmt.pix.width,v4l2_fmt_temp.fmt.pix.height);
               currentIndex++;
               last_x=v4l2_fmt_temp.fmt.pix.width;
               last_y=v4l2_fmt_temp.fmt.pix.height;
               cout << "Adding " << last_x << "x" << last_y << endl;
            }
         } else
            // VIDIOC_TRY_FMT not supported
            // v4l2
            if (-1 == ioctl(device_,VIDIOC_S_FMT,&v4l2_fmt_temp)){
               // ... and store it if it as changed
               if((last_x!=v4l2_fmt_temp.fmt.pix.width)||(last_y!=v4l2_fmt_temp.fmt.pix.height)) {
                  sizeTable_[currentIndex]=QSize(v4l2_fmt_temp.fmt.pix.width,v4l2_fmt_temp.fmt.pix.height);
                  currentIndex++;
                  last_x=v4l2_fmt_temp.fmt.pix.width;
                  last_y=v4l2_fmt_temp.fmt.pix.height;
                  cout << "Adding " << last_x << "x" << last_y << endl;
               }
            else perror("ioctl (VIDIOC_S_FMT)");
         }
         // reducing size by half
         v4l2_fmt_temp.fmt.pix.width/=2;
         v4l2_fmt_temp.fmt.pix.height/=2;
         sizeTable_[currentIndex]=QSize(0,0);
      }
      cout << endl;
   }
   return sizeTable_;
}

// change the frame size
bool QCamV4L::setSize(int x, int y) {
   static char buff[11];
   struct v4l2_crop cropBounds;

   cropBounds.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
   // release mapped mem
   if(useMmap)
      mmapRelease();
   // trying the size
   //cout << "resizing : x=" << x << " " << "y=" << y << endl;

   // if max size, nothing to crop or bin.
   if((x==maxWidth)&&(y==maxHeight))
      croppingMode=SCALING;
   // if hardware cropping not supported, use software cropping
   if(!(options_ & supportCropping)&&(croppingMode==CROPPING)) {
      cout << "using software cropping" << endl;
      croppingMode=CROPPING_SOFT;
   }

   // size to have after cropping or binnning
   targetWidth=x;
   targetHeight=y;

   switch(croppingMode) {
      // scaling : set wanted size
      case SCALING :
      case CROPPING :
         v4l2_fmt_.fmt.pix.width=targetWidth;
         v4l2_fmt_.fmt.pix.height=targetHeight;
         break;
      // soft cropping & binning : set max size
      case CROPPING_SOFT :
      case BINNING :
         v4l2_fmt_.fmt.pix.width=maxWidth;
         v4l2_fmt_.fmt.pix.height=maxHeight;
         x=maxWidth;
         y=maxHeight;
         break;
   }
   // v4l2
   if(ioctl(device_, VIDIOC_S_FMT, &v4l2_fmt_))
   // reading the new size
   // v4l2
   if(ioctl(device_, VIDIOC_G_FMT, &v4l2_fmt_)) {
      perror ("ioctl(VIDIOC_G_FMT)");
   }
   // if the two sizes are diffrent, the device does not
   // support hot resizing, closing and re-opening device
   // the device allready had all these settings, assuming
   // everything goes well...
   if((x!=v4l2_fmt_.fmt.pix.width)||(y!=v4l2_fmt_.fmt.pix.height)) {
      close(device_);
      device_=open(devpath_.c_str(),O_RDONLY | ((options_ & ioNoBlock)?O_NONBLOCK:0));
      // setting the source back
      // v4l2
      ioctl(device_, VIDIOC_S_INPUT, &deviceSource);
      // setting the palette back
      // v4l2
      ioctl(device_, VIDIOC_S_FMT, &v4l2_fmt_);
      // setting the size back
      switch(croppingMode) {
         // scaling : set wanted size
         case SCALING :
         case CROPPING :
            v4l2_fmt_.fmt.pix.width=targetWidth;
            v4l2_fmt_.fmt.pix.height=targetHeight;
            break;
         // soft cropping & binning : set max size
         case CROPPING_SOFT :
         case BINNING :
            v4l2_fmt_.fmt.pix.width=maxWidth;
            v4l2_fmt_.fmt.pix.height=maxHeight;
            break;
      }
      // v4l2
      ioctl(device_, VIDIOC_S_FMT, &v4l2_fmt_);
   }
   // cropping
   switch(croppingMode) {
      // cropping soft : nothing to do
      case CROPPING_SOFT :
         break;
      // scaling & binning : set default cropping rect.
      case SCALING :
         if(!(options_ & supportCropping)) break;
      case BINNING :
         if(ioctl(device_,VIDIOC_CROPCAP,&v4l2_crop_)==-1) {
            // disabling the comboBox
            //cropCombo->update(SCALING);
            //cropCombo->setEnabled(false);
            options_&=~supportCropping;
            cout << "trouble while cropping" << endl;
         } else
            cropBounds.c=v4l2_crop_.defrect;
         break;
      // cropping : set wanted window
      case CROPPING :
         cropBounds.c.left=(maxWidth-x)/2;
         cropBounds.c.top=(maxHeight-y)/2;
         cropBounds.c.width=x;
         cropBounds.c.height=y;
         if(ioctl(device_,VIDIOC_S_CROP,&cropBounds)==-1) {
            // disabling the comboBox
            //cropCombo->update(SCALING);
            //cropCombo->setEnabled(false);
            options_&=~supportCropping;
            cout << "trouble while cropping" << endl;
         }
         break;
   }
   // updating video stream properties
   snprintf(buff,10,"%dx%d",targetWidth,targetHeight);
   setProperty("FrameSize",buff,true);
   // realloc buffers using new size
   allocBuffers();
   // updating mmap
   if(useMmap)
      useMmap=mmapInit();
   // setting controls back
   updatePictureSettings();
   return(true);
}

// drop frames without treatment
bool QCamV4L::dropFrame() {
   ssize_t tmp;
   uchar* nullBuff=NULL;
   // mmap case
   if (useMmap) {
      nullBuff=mmapCapture();
      return(true);
   }
   // else, allocates memory
   nullBuff=(uchar*)malloc(yuvFrameMemSize);
   // read the frame
   tmp=read(device_,(void*)nullBuff,yuvFrameMemSize);
   // free memory
   free(nullBuff);

   return(true);
}

// we should have a new frame
bool QCamV4L::updateFrame() {
   static char nullBuf[720*576];
   bool res;
   double currentTime;

   void * YBuf=NULL,*UBuf=NULL,*VBuf=NULL;
   YBuf=(void*)inputBuffer_.YforOverwrite();
   // compute raw modes (conversions)
   switch(mode_) {
      case GreyFrame:
      case RawRgbFrame1:
      case RawRgbFrame2:
      case RawRgbFrame3:
      case RawRgbFrame4:
         UBuf=VBuf=nullBuf;
         break;
      case YuvFrame:
         UBuf=(void*)inputBuffer_.UforOverwrite();
         VBuf=(void*)inputBuffer_.VforOverwrite();
   }
   inputBuffer_.setMode(mode_);

   // if we are using mmap
   if (useMmap) {
      tmpBuffer_=mmapCapture();
      res=true;
   } else {
   // else we read the device
      res= 0 < read(device_,(void*)tmpBuffer_,yuvFrameMemSize);
      if(!res) perror("FrameUpdate");
   }
   // if we have a frame...
   if(res&&(tmpBuffer_!=NULL)) {
      setTime();
      // ...dependings on the palette...
      switch (v4l2_fmt_.fmt.pix.pixelformat) {
         // mem copies
         case V4L2_PIX_FMT_GREY:
            memcpy(YBuf,tmpBuffer_,v4l2_fmt_.fmt.pix.width * v4l2_fmt_.fmt.pix.height);
            break;
         case V4L2_PIX_FMT_YUV420:
            memcpy(YBuf,tmpBuffer_, v4l2_fmt_.fmt.pix.width * v4l2_fmt_.fmt.pix.height);
            memcpy(UBuf,
               tmpBuffer_+ v4l2_fmt_.fmt.pix.width * v4l2_fmt_.fmt.pix.height,
               (v4l2_fmt_.fmt.pix.width/2) * (v4l2_fmt_.fmt.pix.height/2));
            memcpy(VBuf,
               tmpBuffer_+ v4l2_fmt_.fmt.pix.width * v4l2_fmt_.fmt.pix.height+(v4l2_fmt_.fmt.pix.width/2) * (v4l2_fmt_.fmt.pix.height/2),
               (v4l2_fmt_.fmt.pix.width/2) * (v4l2_fmt_.fmt.pix.height/2));
            break;
         // and frame convertions
         case V4L2_PIX_FMT_RGB24:
            ccvt_rgb24_420p(v4l2_fmt_.fmt.pix.width,v4l2_fmt_.fmt.pix.height,
                         tmpBuffer_,
                         YBuf,
                         UBuf,
                         VBuf);
            break;
         case V4L2_PIX_FMT_YUYV:
             ccvt_yuyv_420p(v4l2_fmt_.fmt.pix.width,v4l2_fmt_.fmt.pix.height,
                         tmpBuffer_,
                         YBuf,
                         UBuf,
                         VBuf);
            break;
         default:
            cerr << "invalid palette " << endl;
            exit(1);
      }
      // debayer the input frame if needed
      if((mode_!=GreyFrame)&&(mode_!=YuvFrame))
         inputBuffer_.debayer();
      // apply software resizing if needed
      switch(croppingMode) {
         case CROPPING_SOFT :
            outputBuffer_.cropping(inputBuffer_,(maxWidth-targetWidth)/2,(maxHeight-targetHeight)/2,targetWidth,targetHeight);
            break;
         case BINNING :
            outputBuffer_.binning(inputBuffer_,targetWidth,targetHeight);
            break;
         default :
            outputBuffer_=inputBuffer_;
            break;
      }
   }
   // if mmap, restoring tmpBuffer_
   if(useMmap)
      tmpBuffer_=NULL;
   // lx support
   // V4L generic may use very diffrent devices, and we can't know the number
   // of frame we should drop. So, we use a threshold tip. We fix a threshold
   // value with a slider, and all the frames with an average luminance below
   // the threshold will be dropped.
   if (res) {
      // if lx activated
      if(lxEnabled) {
         // count dropped frames
         lxFrameCounter++;
         // update progress bar
         if(lxBar) lxBar->setProgress(lxFrameCounter);
         // is there an image on this frame ?
         if(!outputBuffer_.isValide(lxLevel)) {
            // frame not valide
            // if too much frames dropped, we missed the good frame, resetting
            if(lxFrameCounter>(int)(lxDelay*(double)(frameRate_)+4))
               lxFrameCounter=0;
            // ignoring frame
            //cout << "frame dropped" << endl;
            return(0);
         }
         // the frame is ok
         // resetting dropped frames counter
         lxFrameCounter=0;
         // resetting progress bar
         lxBar->reset();
         // blinking
         if(lxBlink) lxBlink->step();
      }
      newFrameAvaible();
        if (options_ & haveBrightness) emit brightnessChange(getBrightness());
        if (options_ & haveContrast) emit contrastChange(getContrast());
        if (options_ & haveHue) emit hueChange(getHue());
        if (options_ & haveColor) emit colorChange(getColor());
        if (options_ & haveWhiteness) emit whitenessChange(getWhiteness());
   }
   int newFrameRate=getFrameRate();
   if (frameRate_ != newFrameRate) {
      frameRate_=newFrameRate;
      //if (timer_) timer_->changeInterval(1000/frameRate_);
   }
   return res;
}

const QSize & QCamV4L::size() const {
   return outputBuffer_.size();
}

void QCamV4L::setContrast(int val) {
   picture_.contrast=val;
   updatePictureSettings();
}

int QCamV4L::getContrast() const {
   return(picture_.contrast);
}

void QCamV4L::setBrightness(int val) {
   picture_.brightness=val;
   updatePictureSettings();
}

int QCamV4L::getBrightness() const {
   return(picture_.brightness);
}

void QCamV4L::setColor(int val) {
   picture_.colour=val;
   updatePictureSettings();
}

int QCamV4L::getColor() const {
   return(picture_.colour);
}

void QCamV4L::setHue(int val) {
   picture_.hue=val;
   updatePictureSettings();
}

int QCamV4L::getHue() const {
   return(picture_.hue);
}

void QCamV4L::setWhiteness(int val) {
   picture_.whiteness=val;
   updatePictureSettings();
}

int QCamV4L::getWhiteness() const {
   return(picture_.whiteness);
}

QCamV4L::~QCamV4L() {
   // release buffer mem if needed
   if(tmpBuffer_!=NULL)
      delete tmpBuffer_;
   // release mmap zone i needed
   if(useMmap)
      mmapRelease();
   // close the video device
   close(device_);
}

void QCamV4L::updatePictureSettings() {
   v4l2_control ctrl;

   // set properties
   // brightness
   ctrl.id=V4L2_CID_BRIGHTNESS;
   ctrl.value=picture_.brightness;
   // v4l2
   ioctl(device_,VIDIOC_S_CTRL,&ctrl);
   // hue
   ctrl.id=V4L2_CID_HUE;
   ctrl.value=picture_.hue;
   // v4l2
   ioctl(device_,VIDIOC_S_CTRL,&ctrl);
   // saturation
   ctrl.id=V4L2_CID_SATURATION;
   ctrl.value=picture_.colour;
   // v4l2
   ioctl(device_,VIDIOC_S_CTRL,&ctrl);
   // contrast
   ctrl.id=V4L2_CID_CONTRAST;
   ctrl.value=picture_.contrast;
   // v4l2
   ioctl(device_,VIDIOC_S_CTRL,&ctrl);
   // whiteness
   ctrl.id=V4L2_CID_WHITENESS;
   ctrl.value=picture_.whiteness;
   // v4l2
   ioctl(device_,VIDIOC_S_CTRL,&ctrl);

   // get properties
   // brightness
   ctrl.id=V4L2_CID_BRIGHTNESS;
   // v4l2
   ioctl(device_,VIDIOC_G_CTRL,&ctrl);
   picture_.brightness=ctrl.value;
   // hue
   ctrl.id=V4L2_CID_HUE;
   // v4l2
   ioctl(device_,VIDIOC_G_CTRL,&ctrl);
   picture_.hue=ctrl.value;
   // saturation
   ctrl.id=V4L2_CID_SATURATION;
   // v4l2
   ioctl(device_,VIDIOC_G_CTRL,&ctrl);
   picture_.colour=ctrl.value;
   // contrast
   ctrl.id=V4L2_CID_CONTRAST;
   // v4l2
   ioctl(device_,VIDIOC_G_CTRL,&ctrl);
   picture_.contrast=ctrl.value;
   // whiteness
   ctrl.id=V4L2_CID_WHITENESS;
   // v4l2
   ioctl(device_,VIDIOC_G_CTRL,&ctrl);
   picture_.whiteness=ctrl.value;
}

void QCamV4L::refreshPictureSettings() {
   v4l2_control ctrl;
   // get properties
   // brightness
   ctrl.id=V4L2_CID_BRIGHTNESS;
   // v4l2
   ioctl(device_,VIDIOC_G_CTRL,&ctrl);
   picture_.brightness=ctrl.value;
   // hue
   ctrl.id=V4L2_CID_HUE;
   // v4l2
   ioctl(device_,VIDIOC_G_CTRL,&ctrl);
   picture_.hue=ctrl.value;
   // saturation
   ctrl.id=V4L2_CID_SATURATION;
   // v4l2
   ioctl(device_,VIDIOC_G_CTRL,&ctrl);
   picture_.colour=ctrl.value;
   // contrast
   ctrl.id=V4L2_CID_CONTRAST;
   // v4l2
   ioctl(device_,VIDIOC_G_CTRL,&ctrl);
   picture_.contrast=ctrl.value;
   // whiteness
   ctrl.id=V4L2_CID_WHITENESS;
   // v4l2
   ioctl(device_,VIDIOC_G_CTRL,&ctrl);
   picture_.whiteness=ctrl.value;

   if (options_ & haveBrightness) emit brightnessChange(getBrightness());
   if (options_ & haveContrast) emit contrastChange(getContrast());
   if (options_ & haveHue) emit hueChange(getHue());
   if (options_ & haveColor) emit colorChange(getColor());
   if (options_ & haveWhiteness) emit whitenessChange(getWhiteness());
}

QWidget * QCamV4L::buildGUI(QWidget * parent) {
   QWidget * remoteCTRL=QCam::buildGUI(parent);
   QGridBox * hbox= new QGridBox(remoteCTRL,Qt::Vertical,3);

   // if cropping not supported, disabling comboBox
   //if(!(options_ & supportCropping))
   //   cropCombo->setEnabled(false);

   int labelNumber;
   if(v4l2_fmt_.fmt.pix.pixelformat==V4L2_PIX_FMT_GREY)
      labelNumber=1;
   else
      labelNumber=6;

   // raw mode settings box
   int frameModeTable[]={GreyFrame,YuvFrame,RawRgbFrame1,RawRgbFrame2,RawRgbFrame3,RawRgbFrame4};
   const char* frameModeLabel[]={"Grey", "RGB", "Raw color GR","Raw color RG (Vesta)","Raw color BG (TUC)","Raw color GB"};
   infoBox=new QHGroupBox(tr("Source"),remoteCTRL);
   frameModeB= new QCamComboBox("frame type",infoBox,labelNumber,frameModeTable,frameModeLabel);
   frameModeB->setMaximumWidth(136);
   connect(frameModeB,SIGNAL(change(int)),this,SLOT(setMode(int)));
   // looking for a saved raw mode
   if(settings.haveKey("RAW_MODE")) {
	int index=frameModeB->getPosition(settings.getKey("RAW_MODE"));
	if (index!=-1) setMode(index);
	frameModeB->setCurrentItem(index);
   // else use default
   } else setMode(0);

   if (options_ & haveContrast) {
      remoteCTRLcontrast_=new QCamSlider("Cont.",false,hbox);
      remoteCTRLcontrast_->setMinValue(picture_.contrast_min);
      remoteCTRLcontrast_->setMaxValue(picture_.contrast_max);
      remoteCTRLcontrast_->setValue(picture_.contrast);
      connect(this,SIGNAL(contrastChange(int)),
              remoteCTRLcontrast_,SLOT(setValue(int)));
      connect(remoteCTRLcontrast_,SIGNAL(valueChange(int)),
              this,SLOT(setContrast(int)));
   }
   if (options_ & haveBrightness) {
      remoteCTRLbrightness_=new QCamSlider("Bri.",false,hbox);
      remoteCTRLbrightness_->setMinValue(picture_.brightness_min);
      remoteCTRLbrightness_->setMaxValue(picture_.brightness_max);
      remoteCTRLbrightness_->setValue(picture_.brightness);
      connect(this,SIGNAL(brightnessChange(int)),
              remoteCTRLbrightness_,SLOT(setValue(int)));
      connect(remoteCTRLbrightness_,SIGNAL(valueChange(int)),
              this,SLOT(setBrightness(int)));
   }
   if (options_ & haveHue) {
      remoteCTRLhue_=new QCamSlider("Hue",false,hbox);
      remoteCTRLhue_->setMinValue(picture_.hue_min);
      remoteCTRLhue_->setMaxValue(picture_.hue_max);
      remoteCTRLhue_->setValue(picture_.hue);
      connect(this,SIGNAL(hueChange(int)),
              remoteCTRLhue_,SLOT(setValue(int)));
      connect(remoteCTRLhue_,SIGNAL(valueChange(int)),
              this,SLOT(setHue(int)));
   }
   if (options_ & haveColor) {
      remoteCTRLcolor_=new QCamSlider("Col.",false,hbox);
      remoteCTRLcolor_->setMinValue(picture_.colour_min);
      remoteCTRLcolor_->setMaxValue(picture_.colour_max);
      remoteCTRLcolor_->setValue(picture_.colour);
      connect(this,SIGNAL(colorChange(int)),
              remoteCTRLcolor_,SLOT(setValue(int)));
      connect(remoteCTRLcolor_,SIGNAL(valueChange(int)),
              this,SLOT(setColor(int)));
   }
   if (options_ & haveWhiteness) {
      remoteCTRLwhiteness_=new QCamSlider("Whit.",false,hbox);
      remoteCTRLwhiteness_->setMinValue(picture_.whiteness_min);
      remoteCTRLwhiteness_->setMaxValue(picture_.whiteness_max);
      remoteCTRLwhiteness_->setValue(picture_.whiteness);
      connect(this,SIGNAL(whitenessChange(int)),
              remoteCTRLwhiteness_,SLOT(setValue(int)));
      connect(remoteCTRLwhiteness_,SIGNAL(valueChange(int)),
              this,SLOT(setWhiteness(int)));
   }

   // level slider
   lxSlider=new QCamSlider("Lvl.",false,hbox,0,255);
   lxSlider->setValue(lxLevel);
   connect(lxSlider,SIGNAL(valueChange(int)),this,SLOT(LXlevel(int)));
   QToolTip::add(lxSlider,"Dropping frame level");

   // palette and input display
   infoLabel1=new QLabel(infoBox);
   infoLabel1->setText("Input :");
   infoLabel1->setMaximumWidth(48);
   infoInput=new QLabel(infoBox);
   infoInput->setAlignment(AlignLeft|AlignVCenter);
   infoInput->setText((char*)input.name);
   infoLabel2=new QLabel(infoBox);
   infoLabel2->setText("Palette :");
   infoLabel2->setMaximumWidth(56);
   infoPalette=new QLabel(infoBox);
   infoPalette->setText(palette);
   infoPalette->setAlignment(AlignLeft|AlignVCenter);
   // Tips
   QToolTip::add(infoInput,"V4L input used");
   QToolTip::add(infoPalette,"V4L palette used");

   // V4L generic long exposure
   // container
   remoteCTRLlx= new QHGroupBox(tr("long exposure"),remoteCTRL);
   // frame rate display
   lxLabel1= new QLabel("fps :",remoteCTRLlx);
   lxRate= new QLabel(QString().sprintf("%i",frameRate_),remoteCTRLlx);
   lxRate->setAlignment(AlignLeft|AlignVCenter);
   lxRate->setMinimumWidth(32);
   // lx mode selector
   int lxTable[]={lxNone,lxPar,lxSer};
   const char* lxLabel[]={"lx : none","lx : // port","lx : serial"};
   lxSelector=new QCamComboBox(tr("lxMode"),remoteCTRLlx,3,lxTable,lxLabel);
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
   lxBar->setCenterIndicator(true);
   lxBar->setTotalSteps(0);
   lxBar->reset();
   // blink
   lxBlink=new QBlink(remoteCTRLlx);
   // tips
   QToolTip::add(lxRate,"Current frame rate");
   QToolTip::add(lxSelector,"Long exposure mode");
   QToolTip::add(lxTime,"Integration time in seconds (0.2s steps)");
   QToolTip::add(lxSet,"Set integration time");
   QToolTip::add(lxBar,"Integration progress bar");
   // default lx delay, init
   lxDelay=0.2;
   lxControler=NULL;
   // lx events connector
   connect(lxSelector,SIGNAL(change(int)),this,SLOT(setLXmode(int)));
   connect(lxSet,SIGNAL(released()),this,SLOT(setLXtime()));

   return remoteCTRL;
}

// changing raw mode
void  QCamV4L::setMode(int  val) {
   QString value=frameModeB->text(val);
   settings.setKey("RAW_MODE",value.data());
   setMode((ImageMode)val);
}

// changing raw mode
void  QCamV4L::setMode(ImageMode val) {
   switch (val) {
   case YuvFrame:
      if (v4l2_fmt_.fmt.pix.pixelformat != V4L2_PIX_FMT_GREY) {
         // color mode possible only if palette is not B&W
         mode_=val;
      }
      break;
   case GreyFrame:
   case RawRgbFrame1:
   case RawRgbFrame2:
   case RawRgbFrame3:
   case RawRgbFrame4:
      mode_=val;
      break;
   }
   inputBuffer_.setMode(mode_);
}

// setting lx modes
void QCamV4L::setLXmode(int value) {
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
         lxBar->setTotalSteps(0);
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
         lxTimer->start((int)((lxDelay/*+lxFineDelay*/)*1000));
         lxControler->enterLongPoseMode();
         lxFrameCounter=0;
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
         lxControler->startAccumulation();
         break;
   }
}

// changing integration time
void QCamV4L::setLXtime() {
   float val;
   // reading edit line and converts
   QString str=lxTime->text();
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
   lxBar->setTotalSteps((int)(lxDelay*frameRate_));
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
void QCamV4L::LXframeReady() {
   // stop integration
   lxControler->stopAccumulation();
   // wait for the canera to send the frames
   // worst case : we must wait for 2 complet fields
   usleep((int)(1000000.0/(1.25*frameRate_)));
   // integration starting for the next frame
   lxControler->startAccumulation();
}

// lx level slider change slot
void QCamV4L::LXlevel(int level) {
   lxLevel=level;
}

// mmap init
bool QCamV4L::mmapInit() {
   // setting struct
   mmap_reqbuf.count=BUFF_NUMBER;

   // request buffs
   if(ioctl(device_,VIDIOC_REQBUFS, &mmap_reqbuf)==-1) {
      cout << "Troubles with video streaming, using read/write" << endl;
      mmap_reqbuf.count=-1;
      mmapRelease();
      return(false);
   }
   //cout << "Using " << mmap_reqbuf.count << " buffers for video streaming" << endl;

   // alloc buffers table
   buffers=(struct mmap_buffer*)calloc(mmap_reqbuf.count, sizeof(struct mmap_buffer));
   if(buffers==NULL) {
      cout << "Mmap memory allocation fails, using read/write" << endl;
      mmapRelease();
      return(false);
   }
   // mmap and enqueue each buffer
   for(int i=0;i<mmap_reqbuf.count;i++) {
      struct v4l2_buffer buffer;
      memset(&buffer,0,sizeof(v4l2_buffer));
      buffer.type=mmap_reqbuf.type;
      buffer.memory=mmap_reqbuf.memory;
      buffer.index=i;
      // query buf
      if(ioctl(device_, VIDIOC_QUERYBUF, &buffer)==-1) {
         cout << "Mmap buffers allocation fails, using read/write" << endl;
         buffers[i].start=MAP_FAILED;
         mmap_reqbuf.count=i;
         mmapRelease();
         return(false);
      }
      // mmap buf
      buffers[i].length=buffer.length;
      buffers[i].start=mmap(NULL, buffer.length, PROT_READ,MAP_SHARED,device_,buffer.m.offset);
      if(buffers[i].start==MAP_FAILED) {
         perror("mmap");
         cout << "Mmap mapping fails, using read/write" << endl;
         mmap_reqbuf.count=i;
         mmapRelease();
         return(false);
      }
      //enqueue buffer
      if(ioctl(device_,VIDIOC_QBUF,&buffer)) {
         cout << "Mmap buffer queuing fails, using read/write" << endl;
         mmap_reqbuf.count=i;
         mmapRelease();
         return(false);
      }
   }

   // start streaming
   if(ioctl(device_,VIDIOC_STREAMON,&mmap_reqbuf.type)) {
      cout << "Mmap stream start fails, using read/write" << endl;
      mmapRelease();
      return(false);
   }

   return(true);
}

// mmap capture
uchar* QCamV4L::mmapCapture() {
   static v4l2_buffer buffer;
   buffer.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
   buffer.memory=V4L2_MEMORY_MMAP;
   // enqueue previous buffer
   if(ioctl(device_, VIDIOC_QBUF,&buffer)!=0)
   // dequeue new buffer
   if(ioctl(device_, VIDIOC_DQBUF,&buffer)!=0)
      perror("DQBUF");

   return((uchar*)buffers[buffer.index].start);
}

void QCamV4L::mmapRelease() {
   // stop streaming
   ioctl(device_,VIDIOC_STREAMOFF,&mmap_reqbuf.type);
   // release buffers, mem, etc.
   if(mmap_reqbuf.count!=-1) {
      //unmap each buffer
      if(buffers!=NULL) {
         for(int i=0;i<mmap_reqbuf.count;i++) {
            if(buffers[i].start!=MAP_FAILED)
               munmap(buffers[i].start,buffers[i].length);
         }
         free(buffers);
         buffers=NULL;
      }
      // release driver buffers
      mmap_reqbuf.count=0;
      ioctl(device_,VIDIOC_REQBUFS, &mmap_reqbuf);
   }
}

// gives os time in second (usec accuracy)
double QCamV4L::getTime() {
   double t;
   struct timeval tv;
   gettimeofday(&tv,NULL);
   t=(float)tv.tv_usec/(float)1000000;
   t+=tv.tv_sec;
   return(t);
}
