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
#include <qmessagebox.h>

#include "QCamSlider.hpp"
#include "yuv.hpp"
#include "QGridBox.hpp"
#include "QCamComboBox.hpp"
#include "SettingsBackup.hpp"

#include "SCmodParPortPPdev.hpp"

// supported palettes
struct palette_datas supported_palettes[]={
   {V4L2_PIX_FMT_RGB24,3,1,"rgb24",YuvFrame},
   {V4L2_PIX_FMT_YUYV,2,1,"yuyv",YuvFrame},
   {V4L2_PIX_FMT_YUV420,3,2,"yuv420",YuvFrame},
   {V4L2_PIX_FMT_GREY,1,1,"grey",GreyFrame},
   {V4L2_PIX_FMT_SBGGR8,1,1,"BA81",GreyFrame},
   {V4L2_PIX_FMT_JPEG,3,1,"jpeg",YuvFrame},
   //{V4L2_PIX_FMT_PWC1,3,1,"philips raw",YuvFrame},
   //{V4L2_PIX_FMT_PWC2,3,1,"philips raw",YuvFrame},
   {-1,0,0,"",0}
};

// find supported tab index depending on the v4l2 palette name/num
// returns -1 if not found
int getSupportedPaletteIndex(int v4l2_palette) {
   int index=0;
   while((supported_palettes[index].index!=-1)&&(supported_palettes[index].index!=v4l2_palette))
      index++;
   if(supported_palettes[index].index==-1)
      return(-1);
   return(index);
}

// the same with palette name
int getSupportedPaletteIndex(const char* palette_name) {
   int index=0;
   while((supported_palettes[index].index!=-1)&&(strcmp(supported_palettes[index].name,palette_name)!=0))
      index++;
   if(supported_palettes[index].index==-1)
      return(-1);
   return(index);
}

// settings object, needed everywhere
extern settingsBackup settings;

// static value init
const int QCamV4L::DefaultOptions=(haveBrightness|haveContrast|haveHue|haveColor|haveWhiteness);

// constructor
QCamV4L::QCamV4L(const char * devpath, unsigned long options /* cf QCamV4L::options */) {
   // V4L2 needed vars
   v4l2_std_id _id; // video stream standard id
   v4l2_standard standard; // video stream standard
   v4l2_fmtdesc fmtdesc;
   int _index=0;
   // palettes vars
   //
   paletteNumber=0;
   // init defaults value
   palette=0;
   options_=options;
   tmpBuffer_=NULL;
   remoteCTRLbrightness_=NULL;
   remoteCTRLcontrast_=NULL;
   remoteCTRLhue_=NULL;
   remoteCTRLcolor_=NULL;
   remoteCTRLwhiteness_=NULL;
   sizeTable_=NULL;
   jpegImageBuffer=NULL;
   jpegCopyBuffer=NULL;
   jpegLineBuffer[0]=NULL;
   device_=-1;
   devpath_=devpath;
   mode_=(ImageMode)0;
   memset(&v4l2_fmt_,0,sizeof(v4l2_format));
   memset(&v4l2_cap_,0,sizeof(v4l2_capability));
   memset(&_id,0,sizeof(v4l2_std_id));
   memset(&standard,0,sizeof(v4l2_standard));

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
   memset(&qctrl,0,sizeof(v4l2_queryctrl));
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
   memset(&ctrl,0,sizeof(v4l2_control));
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
   memset(&input,0,sizeof(v4l2_input));
   input.index=0;
   sourceNumber=0;
   // v4l2
   while(!ioctl(device_,VIDIOC_ENUMINPUT,&input)) {
      sourceTable[sourceNumber]=input.index;
      sourceLabel[sourceNumber]=(char*)malloc(32);
      memcpy((void*)sourceLabel[sourceNumber],input.name,32);
      cout << " - " << input.name << endl;
      input.index++;
      sourceNumber++;
   }
   // choosing stored source if there is one
   // the storing key is "SOURCE"+<device_name>
   // so, we can store a prefered source for each diffrent device
   int res;
   string keyName("SOURCE_");
   keyName+=(char*)v4l2_cap_.card;
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
   }
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
   // ****************************
   // enumerate available palettes
   // ****************************
   cout << "available palettes :" << endl;
   memset(&fmtdesc,0,sizeof(v4l2_fmtdesc));
   fmtdesc.index=0;
   fmtdesc.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
   while(!ioctl(device_,VIDIOC_ENUM_FMT,&fmtdesc)) {
      int index=getSupportedPaletteIndex(fmtdesc.pixelformat);
      if(index!=-1) {
         paletteTable[paletteNumber]=paletteNumber;
         paletteLabel[paletteNumber]=(char*)malloc(32);
         memcpy((void*)paletteLabel[paletteNumber],supported_palettes[index].name,32);
         cout << " - supported palette :  " << supported_palettes[index].name << endl;
         paletteNumber++;
         palette=index;
      }
      else
         cout << " - unsupported palette : " << fmtdesc.description << endl;
      fmtdesc.index++;
   }
   if(paletteNumber==0) {
      QMessageBox::information(0,"Qastrocam-g2","No supported palette found\nLeaving...");
      cout << "No supported palette found, living " << endl;
      exit(1);
   }
   // *************************
   // palette setting
   // *************************
   string keyName2("PALETTE_");
   keyName2+=(char*)v4l2_cap_.card;
   if(settings.haveKey(keyName2.c_str())) {
      // we have a stored prefered palette
      cout << "found stored palette : " << settings.getKey(keyName2.c_str()) << endl ;
      string palette_name=settings.getKey(keyName2.c_str());
      int index=getSupportedPaletteIndex(palette_name.c_str());
      if(index!=-1)
         palette=index;
   }
   updatePalette();
   cout << "using : " << supported_palettes[palette].name << endl << endl ;
   // ********************************************
   // getting video standard to compute frame rate
   // ********************************************
   // v4l2
   if(ioctl(device_,VIDIOC_G_STD,&_id)==-1) {
      perror("Getting Standard");
   }
   // iterate to find the used video standard
   standard.index=0;
   standard.id=-1;
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
   if(frameRate_<=0) {
      cout << "unable to get video frame rate" << endl;
      // try to get framerate for pwc
      struct video_window window_;
      memset(&window_,0,sizeof(struct video_window));
      // v4l
      if(ioctl(device_,VIDIOCGWIN, &window_)==0) {
         frameRate_=(window_.flags&/*PWC_FPS_FRMASK*/0x00FF0000)>>/*PWC_FPS_SHIFT*/16;
         if(frameRate_<=0)
            frameRate_=10;
      } else {
         frameRate_=10;
         cout <<  "Using default Framerate: " << frameRate_ << " fps" << endl;
      }
      cout << endl;
   } else
      cout <<  "Using Framerate : " << frameRate_ << " fps" << endl << endl;
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
      cout << "using software cropping" << endl;
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
         cout << "cropping not supported" << endl << "using software cropping" << endl;
      }
   }

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

void QCamV4L::updatePalette() {
   // getting default values
   v4l2_fmt_.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
   // v4l2
   if(-1 == ioctl(device_,VIDIOC_G_FMT,&v4l2_fmt_))
         perror("ioctl (VIDIOC_G_FMT)");
   v4l2_fmt_.fmt.pix.pixelformat=supported_palettes[palette].index;
   // v4l2
   if (0 == ioctl(device_, VIDIOC_S_FMT, &v4l2_fmt_)) {
      palette=getSupportedPaletteIndex(v4l2_fmt_.fmt.pix.pixelformat);
   }
   // saving palette
   string keyName("PALETTE_");
   keyName+=(char*)v4l2_cap_.card;
   settings.setKey(keyName.c_str(),supported_palettes[palette].name);
}

// allocate memory for buffers, depending on
// frame size and palette
void QCamV4L::allocBuffers() {
   free(tmpBuffer_);
   free(jpegImageBuffer);
   free(jpegCopyBuffer);
   free(jpegLineBuffer[0]);
   inputBuffer_.setSize(QSize(v4l2_fmt_.fmt.pix.width,v4l2_fmt_.fmt.pix.height));
   yuvFrameMemSize=v4l2_fmt_.fmt.pix.width * v4l2_fmt_.fmt.pix.height * supported_palettes[palette].memfactor_numerator / supported_palettes[palette].memfactor_denominator;
   tmpBuffer_=(unsigned char*)malloc(yuvFrameMemSize);
   // jpeg stuff
   // everything oversized...
   if(supported_palettes[palette].index==V4L2_PIX_FMT_JPEG) {
      row_size=v4l2_fmt_.fmt.pix.width*3;
      jpegImageBuffer=(unsigned char*)malloc(yuvFrameMemSize);
      jpegCopyBuffer=(unsigned char*)malloc(yuvFrameMemSize);
      jpegLineBuffer[0]=(unsigned char *)malloc(row_size);
   }
}

// get frame sizes supported by the
// video device
const QSize * QCamV4L::getAllowedSize() const {
   if (sizeTable_==NULL) {
      int res;
      int currentIndex=0;
      int last_x=0;
      int last_y=0;
      int min_x;
      int min_y;
      sizeTable_=new QSize[16];
      v4l2_fmtdesc v4l2_fmtdesc_temp;
      v4l2_frmsizeenum v4l2_sizeenum_temp;
      v4l2_format v4l2_fmt_temp;
      memset(&v4l2_fmtdesc_temp,0,sizeof(v4l2_fmtdesc));
      memset(&v4l2_sizeenum_temp,0,sizeof(v4l2_frmsizeenum));
      memset(&v4l2_fmt_temp,0,sizeof(v4l2_format));

      cout << "Frame size detection" << endl;

      // get the first pixel fmt
      v4l2_fmtdesc_temp.index=0;
      v4l2_fmtdesc_temp.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
      ioctl(device_,VIDIOC_ENUM_FMT,&v4l2_fmtdesc_temp);
      // get the first frame size
      v4l2_sizeenum_temp.index=0;
      v4l2_sizeenum_temp.pixel_format=v4l2_fmtdesc_temp.pixelformat;
      res=ioctl(device_,VIDIOC_ENUM_FRAMESIZES,&v4l2_sizeenum_temp);
      // if VIDIOC_ENUM_FRAMESIZES supported
      if((res==0)&&(v4l2_sizeenum_temp.type==V4L2_FRMSIZE_TYPE_DISCRETE)) {
         cout << "V4L2 discrete frame enum supported" << endl;
         currentIndex=0;
         while(res==0) {
            sizeTable_[currentIndex]=QSize(v4l2_sizeenum_temp.discrete.width,v4l2_sizeenum_temp.discrete.height);
            currentIndex++;
            sizeTable_[currentIndex]=QSize(0,0);
            cout << " - adding " << v4l2_sizeenum_temp.discrete.width << "x" << v4l2_sizeenum_temp.discrete.height << endl;
            v4l2_sizeenum_temp.index++;
            res=ioctl(device_,VIDIOC_ENUM_FRAMESIZES,&v4l2_sizeenum_temp);
         }
      // else VIDIOC_ENUM_FRAMESIZES discrete not supported
      } else {
         cout << "V4L2 discrete frame enum not supported" << endl;
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
         while((currentIndex<15)&&(v4l2_fmt_temp.fmt.pix.width>=min_x)&&(v4l2_fmt_temp.fmt.pix.height>=min_y)) {
            // try the new size...
            // v4l2
            if (ioctl(device_, VIDIOC_TRY_FMT, &v4l2_fmt_temp)!=-1) {
               // ... and store it if it as changed
               if((last_x!=v4l2_fmt_temp.fmt.pix.width)||(last_y!=v4l2_fmt_temp.fmt.pix.height)) {
                  sizeTable_[currentIndex]=QSize(v4l2_fmt_temp.fmt.pix.width,v4l2_fmt_temp.fmt.pix.height);
                  currentIndex++;
                  last_x=v4l2_fmt_temp.fmt.pix.width;
                  last_y=v4l2_fmt_temp.fmt.pix.height;
                  cout << " - adding " << last_x << "x" << last_y << endl;
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
                     cout << " - adding " << last_x << "x" << last_y << endl;
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
      //cout << "using software cropping" << endl;
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
   unsigned char* nullBuf;
   unsigned char* oldTmpBuffer_;
   bool res;
   double currentTime;

   if(lxEnabled) {
      if(lxFramesToDrop>1) {
         // we still drop frames
         lxFrameCounter++;
         // update progress bar
         if(lxBar) lxBar->setProgress(lxFrameCounter);
         dropFrame();
         return(true);
      } else if (lxFramesToDrop==1) {
         // last frame to drop
         lxFramesToDrop--;
         lxFrameCounter++;
         // update progress bar
         if(lxBar) lxBar->setProgress(lxFrameCounter);
         dropFrame();
         return(true);
      } else {
         // we have a frame, reset for the next period;
         lxFramesToDrop=2;
         // resetting dropped frames counter
         lxFrameCounter=0;
         // resetting progress bar
         lxBar->reset();
         lxControler->startAccumulation();
      }
   }

   nullBuf=(unsigned char*)malloc(v4l2_fmt_.fmt.pix.width * v4l2_fmt_.fmt.pix.height);
   unsigned char * YBuf=NULL,*UBuf=NULL,*VBuf=NULL;
   inputBuffer_.setMode(mode_);
   YBuf=(unsigned char*)inputBuffer_.YforOverwrite();
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
         UBuf=(unsigned char*)inputBuffer_.UforOverwrite();
         VBuf=(unsigned char*)inputBuffer_.VforOverwrite();
   }

   // if we are using mmap
   if (useMmap) {
      oldTmpBuffer_=tmpBuffer_;
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
      switch (supported_palettes[palette].index) {
         // mem copies
         case V4L2_PIX_FMT_SBGGR8:
         case V4L2_PIX_FMT_GREY:
            memcpy(YBuf,tmpBuffer_,v4l2_fmt_.fmt.pix.width * v4l2_fmt_.fmt.pix.height);
            break;
         case V4L2_PIX_FMT_YUV420:
            yuv420_to_yuv444(v4l2_fmt_.fmt.pix.width,v4l2_fmt_.fmt.pix.height,
               tmpBuffer_, tmpBuffer_+ v4l2_fmt_.fmt.pix.width*v4l2_fmt_.fmt.pix.height,
               tmpBuffer_+v4l2_fmt_.fmt.pix.width*v4l2_fmt_.fmt.pix.height+(v4l2_fmt_.fmt.pix.width/2)*(v4l2_fmt_.fmt.pix.height/2),YBuf,UBuf,VBuf);
            break;
         // and frame convertions
         case V4L2_PIX_FMT_RGB24:
            rgb24_to_yuv444(v4l2_fmt_.fmt.pix.width,v4l2_fmt_.fmt.pix.height,tmpBuffer_,YBuf,UBuf,VBuf);
            break;
         case V4L2_PIX_FMT_YUYV:
            yuv422_to_yuv444(v4l2_fmt_.fmt.pix.width,v4l2_fmt_.fmt.pix.height,tmpBuffer_,YBuf,UBuf,VBuf);
            break;
         case V4L2_PIX_FMT_JPEG:
            // copy driver buffer to avoid buffer underun
            memcpy(jpegCopyBuffer,tmpBuffer_,v4l2_fmt_.fmt.pix.width*v4l2_fmt_.fmt.pix.height*3);
            // create jpeg object
            cinfo.err = jpeg_std_error(&jerr);
            jpeg_create_decompress(&cinfo);
            jpeg_mem_src(&cinfo,jpegCopyBuffer,yuvFrameMemSize);
            jpeg_read_header(&cinfo, TRUE);
            // output colorspace
            cinfo.out_color_space= JCS_YCbCr;
            jpeg_start_decompress(&cinfo);
            // for each scanline
            row=0;
            while (cinfo.output_scanline < cinfo.output_height) {
               jpeg_read_scanlines(&cinfo, jpegLineBuffer, 1);
               memcpy(&jpegImageBuffer[row*row_size],jpegLineBuffer[0],row_size);
               row++;
            }
            // convert
            ycbcr_to_yuv444(v4l2_fmt_.fmt.pix.width,v4l2_fmt_.fmt.pix.height,jpegImageBuffer,YBuf,UBuf,VBuf);
            // destroy jpeg object
            jpeg_finish_decompress(&cinfo);
            jpeg_destroy_decompress(&cinfo);
            break;
         case V4L2_PIX_FMT_PWC1:
            //
            break;
         case V4L2_PIX_FMT_PWC2:
            //
            break;
         default:
            cout << "invalid palette " << endl;
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
   free(nullBuf);
   // if mmap, restoring tmpBuffer_
   if(useMmap)
      tmpBuffer_=oldTmpBuffer_;
   if (res) {
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
   free(tmpBuffer_);
   // release mmap zone i needed
   if(useMmap)
      mmapRelease();
   // close the video device
   close(device_);

   delete [] sizeTable_;
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
   memset(&ctrl,0,sizeof(v4l2_control));
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

   // source box
   infoBox=new QHGroupBox(tr("Source"),remoteCTRL);

   // palette and input display
   sourceB=new QCamComboBox("source",infoBox,sourceNumber,sourceTable,sourceLabel);
   sourceB->setCurrentText(QString((char*)input.name));
   if(sourceNumber<2)
      sourceB->setEnabled(false);
   connect(sourceB,SIGNAL(change(int)),this,SLOT(setSource(int)));
   paletteB=new QCamComboBox("source",infoBox,paletteNumber,paletteTable,paletteLabel);
   paletteB->setCurrentText(QString(supported_palettes[palette].name));
   if(paletteNumber<2)
      paletteB->setEnabled(false);
   connect(paletteB,SIGNAL(change(int)),this,SLOT(setPalette(int)));
   // tips
   QToolTip::add(sourceB,"V4L2 input used");
   QToolTip::add(paletteB,"V4L2 palette used");

   // frame mode number
   int labelNumber;
   if(supported_palettes[palette].mode==GreyFrame)
      labelNumber=5;
   else
      labelNumber=6;
   // raw mode settings box
   int frameModeTable[]={GreyFrame,RawRgbFrame1,RawRgbFrame2,RawRgbFrame3,RawRgbFrame4,YuvFrame};
   const char* frameModeLabel[]={"Grey", "Raw color GR","Raw color RG (Vesta)","Raw color BG (TUC)","Raw color GB","RGB"};
   // adding frames mode
   frameModeB= new QCamComboBox("frame type",infoBox,labelNumber,frameModeTable,frameModeLabel);
   connect(frameModeB,SIGNAL(change(int)),this,SLOT(setMode(int)));
   // looking for a saved raw mode
   string keyName("RAW_MODE_");
   keyName+=(char*)v4l2_cap_.card;
   if(settings.haveKey(keyName.c_str())) {
        int index=frameModeB->getPosition(settings.getKey(keyName.c_str()));
        if (index!=-1) setMode(index);
        frameModeB->setCurrentItem(index);
   // else use default
   } else setMode(0);
   QToolTip::add(frameModeB,"Frame mode");

   // controls
   QHGroupBox* ctrlBox=new QHGroupBox(tr("Controls"),remoteCTRL);
   QGridBox * hbox= new QGridBox(ctrlBox,Qt::Vertical,3);
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
   padding=new QWidget(remoteCTRL);
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

   // setting up the notifier
   notifier_=NULL;
   // *************************************************
   // notifier (all V4L2 devices must support "select")
   // *************************************************
   notifier_ = new QSocketNotifier(device_, QSocketNotifier::Read, this);
   connect(notifier_,SIGNAL(activated(int)),this,SLOT(updateFrame()));
   //cout << "Using select to wait for new frames.\n" << endl;

   return remoteCTRL;
}

void QCamV4L::setSource(int val) {
   string keyName("SOURCE_");
   keyName+=(char*)v4l2_cap_.card;
   settings.setKey(keyName.c_str(),sourceB->text(val).ascii());
   QMessageBox::information(0,"Qastrocam-g2","Please restart Qastrocam-g2\nto get the new source");
}

// changing palette
void QCamV4L::setPalette(int val) {
   palette=getSupportedPaletteIndex(paletteB->text(val));
   updatePalette();
   allocBuffers();
   // updating mmap
   if(useMmap)
      useMmap=mmapInit();
   // setting controls back
   updatePictureSettings();
}

// changing raw mode
void  QCamV4L::setMode(int  val) {
   QString value=frameModeB->text(val);
   string keyName("RAW_MODE_");
   keyName+=(char*)v4l2_cap_.card;
   settings.setKey(keyName.c_str(),value.data());
   setMode((ImageMode)val);
}

// changing raw mode
void  QCamV4L::setMode(ImageMode val) {
   switch (val) {
   case YuvFrame:
      if (supported_palettes[palette].mode != GreyFrame) {
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
   // last frame to drop
   lxFramesToDrop=1;
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

   cout << "Using mmap with " << mmap_reqbuf.count << " buffers" << endl;
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
