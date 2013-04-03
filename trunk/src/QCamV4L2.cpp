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


#include "QCamV4L2.moc"
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

#define SIZE_TABLE_SLOTS	32

// supported palettes
struct palette_datas supported_palettes[]={
   {V4L2_PIX_FMT_RGB24,3,1,"rgb24",YuvFrame},
   {V4L2_PIX_FMT_UYVY,2,1,"uyvy",YuvFrame},
   {V4L2_PIX_FMT_YUYV,2,1,"yuyv",YuvFrame},
   {V4L2_PIX_FMT_YUV420,3,2,"yuv420",YuvFrame},
   {V4L2_PIX_FMT_GREY,1,1,"grey",GreyFrame},
   {V4L2_PIX_FMT_SBGGR8,1,1,"BA81",GreyFrame},
#if HAVE_JPEG_H
   {V4L2_PIX_FMT_JPEG,3,1,"jpeg",YuvFrame},
#endif
   {V4L2_PIX_FMT_SPCA505,2,1,"s505",YuvFrame},
   //{V4L2_PIX_FMT_SQ905C,1,1,"sq905c",GreyFrame},
#if HAVE_JPEG_H
   //{V4L2_PIX_FMT_MJPEG,3,1,"mjpeg",YuvFrame},
#endif
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
const int QCamV4L2::DefaultOptions=(haveBrightness|haveContrast|haveHue|haveColor|haveWhiteness);

// constructor
QCamV4L2::QCamV4L2(const char * devpath, unsigned long options /* cf QCamV4L::options */) {
   // V4L2 needed vars
   v4l2_std_id _id; // video stream standard id
   v4l2_standard standard; // video stream standard
   v4l2_fmtdesc fmtdesc;
   int _index=0;
   // palettes vars
   //
   paletteNumber=0;
   // init defaults value
   buffNumber=BUFF_NUMBER;
   palette=0;
   options_=options;
   nullBuff=NULL;
   tmpBuffer_=NULL;
   remoteCTRLbrightness_=NULL;
   remoteCTRLcontrast_=NULL;
   remoteCTRLhue_=NULL;
   remoteCTRLcolor_=NULL;
   remoteCTRLwhiteness_=NULL;
   sizeTable_=NULL;
#if HAVE_JPEG_H
   jpegImageBuffer=NULL;
   jpegCopyBuffer=NULL;
   jpegLineBuffer[0]=NULL;
#endif
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
   if (options_ & haveBrightness) {
      // brightness
      ctrl.id=V4L2_CID_BRIGHTNESS;
      // v4l2
      ioctl(device_,VIDIOC_G_CTRL,&ctrl);
      picture_.brightness=ctrl.value;
   }
   if (options_ & haveHue) {
      // hue
      ctrl.id=V4L2_CID_HUE;
      // v4l2
      ioctl(device_,VIDIOC_G_CTRL,&ctrl);
      picture_.hue=ctrl.value;
   }
   if (options_ & haveColor) {
      // saturation
      ctrl.id=V4L2_CID_SATURATION;
      // v4l2
      ioctl(device_,VIDIOC_G_CTRL,&ctrl);
      picture_.colour=ctrl.value;
   }
   if (options_ & haveContrast) {
      // contrast
      ctrl.id=V4L2_CID_CONTRAST;
      // v4l2
      ioctl(device_,VIDIOC_G_CTRL,&ctrl);
      picture_.contrast=ctrl.value;
   }
   if (options_ & haveWhiteness) {
      // whiteness
      ctrl.id=V4L2_CID_WHITENESS;
      // v4l2
      ioctl(device_,VIDIOC_G_CTRL,&ctrl);
      picture_.whiteness=ctrl.value;
   }
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
      cout << "No supported palette found, leaving " << endl;
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
      frameRate_=10;
      cout <<  "Using default Framerate: " << frameRate_ << " fps" << endl;

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
   int i=0;
   maxWidth=0;
   maxHeight=0;
   while((sizeTable[i].width()!=0)&&(sizeTable[i].height()!=0)) {
      if(sizeTable[i].width()>maxWidth) maxWidth=sizeTable[i].width();
      if(sizeTable[i].height()>maxHeight) maxHeight=sizeTable[i].height();
      i++;
   }
   //resize(sizeTable[0]);
   // update video stream properties
   setProperty("CameraName",string((char*)v4l2_cap_.card));
   setProperty("FrameRateSecond",frameRate_);
   label((char*)v4l2_cap_.card);
}

// resize the stream
void QCamV4L2::resize(const QSize & s) {
   setSize(s.width(),s.height());
}

void QCamV4L2::updatePalette() {
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
void QCamV4L2::allocBuffers() {
   free(nullBuff);
   free(tmpBuffer_);
#if HAVE_JPEG_H
   free(jpegImageBuffer);
   free(jpegCopyBuffer);
   free(jpegLineBuffer[0]);
#endif
   inputBuffer_.setSize(QSize(v4l2_fmt_.fmt.pix.width,v4l2_fmt_.fmt.pix.height));
   yuvFrameMemSize=v4l2_fmt_.fmt.pix.width * v4l2_fmt_.fmt.pix.height * supported_palettes[palette].memfactor_numerator / supported_palettes[palette].memfactor_denominator;

   tmpBuffer_=(unsigned char*)malloc(yuvFrameMemSize);
   nullBuff=(unsigned char*)malloc(yuvFrameMemSize);
#if HAVE_JPEG_H
   // jpeg stuff
   // everything oversized...
   if((supported_palettes[palette].index==V4L2_PIX_FMT_JPEG)||(supported_palettes[palette].index==V4L2_PIX_FMT_MJPEG)) {
      row_size=v4l2_fmt_.fmt.pix.width*3;
      jpegImageBuffer=(unsigned char*)malloc(yuvFrameMemSize);
      jpegCopyBuffer=(unsigned char*)malloc(yuvFrameMemSize);
      jpegLineBuffer[0]=(unsigned char *)malloc(row_size);
   }
#endif
}

// get frame sizes supported by the
// video device
const QSize * QCamV4L2::getAllowedSize() const {
   if (sizeTable_==NULL) {
      int res=0;
      int currentIndex=0;
      int last_x=0;
      int last_y=0;
      int min_x;
      int min_y;
      sizeTable_=new QSize[SIZE_TABLE_SLOTS];
      v4l2_fmtdesc v4l2_fmtdesc_temp;
      v4l2_frmsizeenum v4l2_sizeenum_temp;
      v4l2_format v4l2_fmt_temp;
      memset(&v4l2_fmtdesc_temp,0,sizeof(v4l2_fmtdesc));
      memset(&v4l2_sizeenum_temp,0,sizeof(v4l2_frmsizeenum));
      memset(&v4l2_fmt_temp,0,sizeof(v4l2_format));

      cout << "Frame size detection" << endl;

      // get the first supported pixel fmt
      v4l2_fmtdesc_temp.index=0;
      v4l2_fmtdesc_temp.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
      while((res!=-1)&&(getSupportedPaletteIndex(v4l2_fmtdesc_temp.pixelformat)==-1)) {
         res=ioctl(device_,VIDIOC_ENUM_FMT,&v4l2_fmtdesc_temp);
         v4l2_fmtdesc_temp.index++;
      }
      if(res==-1) {
         QMessageBox::information(0,"Qastrocam-g2","No supported palette found\nLeaving...");
         cout << "No supported palette found" << endl;
         exit(1);
      }

      // get the first frame size
      v4l2_sizeenum_temp.index=0;
      v4l2_sizeenum_temp.pixel_format=v4l2_fmtdesc_temp.pixelformat;
      res=ioctl(device_,VIDIOC_ENUM_FRAMESIZES,&v4l2_sizeenum_temp);
      // if VIDIOC_ENUM_FRAMESIZES supported
      if((res==0)&&(v4l2_sizeenum_temp.type==V4L2_FRMSIZE_TYPE_DISCRETE)) {
         cout << "V4L2 discrete frame enum supported" << endl;
         currentIndex=0;
         while((res==0)&&(currentIndex<SIZE_TABLE_SLOTS-1)) {
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
         while((currentIndex<SIZE_TABLE_SLOTS-1)&&(v4l2_fmt_temp.fmt.pix.width>=min_x)&&(v4l2_fmt_temp.fmt.pix.height>=min_y)) {
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
bool QCamV4L2::setSize(int x, int y) {
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
bool QCamV4L2::dropFrame() {
   ssize_t tmp;
   uchar* null=NULL;
   // mmap case
   if (useMmap) {
      null=mmapCapture();
      return(true);
   }
   // read the frame
   tmp=read(device_,(void*)nullBuff,yuvFrameMemSize);
   return(true);
}

// we should have a new frame
bool QCamV4L2::updateFrame() {
   unsigned char* oldTmpBuffer_;
   bool res;
   double currentTime;

   unsigned char * YBuf=NULL,*UBuf=NULL,*VBuf=NULL;
   YBuf=(unsigned char*)inputBuffer_.YforOverwrite();
   // compute raw modes (conversions)
   switch(mode_) {
      case GreyFrame:
         UBuf=VBuf=nullBuff;
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
            if(mode_==YuvFrame)
               yuv420_to_yuv444(v4l2_fmt_.fmt.pix.width,v4l2_fmt_.fmt.pix.height,
                  tmpBuffer_, tmpBuffer_+ v4l2_fmt_.fmt.pix.width*v4l2_fmt_.fmt.pix.height,
                  tmpBuffer_+v4l2_fmt_.fmt.pix.width*v4l2_fmt_.fmt.pix.height+(v4l2_fmt_.fmt.pix.width/2)*(v4l2_fmt_.fmt.pix.height/2),YBuf,UBuf,VBuf);
            else
               yuv420_to_y(v4l2_fmt_.fmt.pix.width,v4l2_fmt_.fmt.pix.height,tmpBuffer_,YBuf);
            break;
         // and frame convertions
         case V4L2_PIX_FMT_RGB24:
            if(mode_==YuvFrame)
               rgb24_to_yuv444(v4l2_fmt_.fmt.pix.width,v4l2_fmt_.fmt.pix.height,tmpBuffer_,YBuf,UBuf,VBuf);
            else
               rgb24_to_y(v4l2_fmt_.fmt.pix.width,v4l2_fmt_.fmt.pix.height,tmpBuffer_,YBuf);
            break;
         case V4L2_PIX_FMT_UYVY:
            if(mode_==YuvFrame)
               uyvy_to_yuv444(v4l2_fmt_.fmt.pix.width,v4l2_fmt_.fmt.pix.height,tmpBuffer_,YBuf,UBuf,VBuf);
            else
               uyvy_to_y(v4l2_fmt_.fmt.pix.width,v4l2_fmt_.fmt.pix.height,tmpBuffer_,YBuf);
            break;
         case V4L2_PIX_FMT_YUYV:
            if(mode_==YuvFrame)
               yuyv_to_yuv444(v4l2_fmt_.fmt.pix.width,v4l2_fmt_.fmt.pix.height,tmpBuffer_,YBuf,UBuf,VBuf);
            else
               yuyv_to_y(v4l2_fmt_.fmt.pix.width,v4l2_fmt_.fmt.pix.height,tmpBuffer_,YBuf);
            break;
#if HAVE_JPEG_H
         case V4L2_PIX_FMT_JPEG:
         case V4L2_PIX_FMT_MJPEG:
            // copy driver buffer to avoid buffer underun
            if(useMmap)
               memcpy(jpegCopyBuffer,tmpBuffer_,buffers[0].length);
            else
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
            if(mode_==YuvFrame)
               ycbcr_to_yuv444(v4l2_fmt_.fmt.pix.width,v4l2_fmt_.fmt.pix.height,jpegImageBuffer,YBuf,UBuf,VBuf);
            else
               ycbcr_to_y(v4l2_fmt_.fmt.pix.width,v4l2_fmt_.fmt.pix.height,jpegImageBuffer,YBuf);
            // destroy jpeg object
            jpeg_finish_decompress(&cinfo);
            jpeg_destroy_decompress(&cinfo);
            break;
#endif
         case V4L2_PIX_FMT_SPCA505:
            if(mode_==YuvFrame)
               s505_to_yuv444(v4l2_fmt_.fmt.pix.width,v4l2_fmt_.fmt.pix.height,tmpBuffer_,YBuf,UBuf,VBuf);
            else
               s505_to_y(v4l2_fmt_.fmt.pix.width,v4l2_fmt_.fmt.pix.height,tmpBuffer_,YBuf);
            break;
         //case V4L2_PIX_FMT_SQ905C:
            //
         //   break;
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
   }
   return res;
}

const QSize & QCamV4L2::size() const {
   return outputBuffer_.size();
}

void QCamV4L2::setContrast(int val) {
   picture_.contrast=val;
   updatePictureSettings();
}

int QCamV4L2::getContrast() const {
   return(picture_.contrast);
}

void QCamV4L2::setBrightness(int val) {
   picture_.brightness=val;
   updatePictureSettings();
}

int QCamV4L2::getBrightness() const {
   return(picture_.brightness);
}

void QCamV4L2::setColor(int val) {
   picture_.colour=val;
   updatePictureSettings();
}

int QCamV4L2::getColor() const {
   return(picture_.colour);
}

void QCamV4L2::setHue(int val) {
   picture_.hue=val;
   updatePictureSettings();
}

int QCamV4L2::getHue() const {
   return(picture_.hue);
}

void QCamV4L2::setWhiteness(int val) {
   picture_.whiteness=val;
   updatePictureSettings();
}

int QCamV4L2::getWhiteness() const {
   return(picture_.whiteness);
}

QCamV4L2::~QCamV4L2() {
   // release buffer mem if needed
   free(tmpBuffer_);
   // release mmap zone i needed
   if(useMmap)
      mmapRelease();
   // close the video device
   close(device_);

   delete [] sizeTable_;
}

void QCamV4L2::updatePictureSettings() {
   v4l2_control ctrl;

   // set properties
   if (options_ & haveBrightness) {
      // brightness
      ctrl.id=V4L2_CID_BRIGHTNESS;
      ctrl.value=picture_.brightness;
      // v4l2
      ioctl(device_,VIDIOC_S_CTRL,&ctrl);
      ioctl(device_,VIDIOC_G_CTRL,&ctrl);
      picture_.brightness=ctrl.value;
   }
   if (options_ & haveHue) {
      // hue
      ctrl.id=V4L2_CID_HUE;
      ctrl.value=picture_.hue;
      // v4l2
      ioctl(device_,VIDIOC_S_CTRL,&ctrl);
      ioctl(device_,VIDIOC_G_CTRL,&ctrl);
      picture_.hue=ctrl.value;
   }
   if (options_ & haveColor) {
      // saturation
      ctrl.id=V4L2_CID_SATURATION;
      ctrl.value=picture_.colour;
      // v4l2
      ioctl(device_,VIDIOC_S_CTRL,&ctrl);
      ioctl(device_,VIDIOC_G_CTRL,&ctrl);
   picture_.colour=ctrl.value;
   }
   if (options_ & haveContrast) {
      // contrast
      ctrl.id=V4L2_CID_CONTRAST;
      ctrl.value=picture_.contrast;
      // v4l2
      ioctl(device_,VIDIOC_S_CTRL,&ctrl);
      ioctl(device_,VIDIOC_G_CTRL,&ctrl);
      picture_.contrast=ctrl.value;
   }
   if (options_ & haveWhiteness) {
      // whiteness
      ctrl.id=V4L2_CID_WHITENESS;
      ctrl.value=picture_.whiteness;
      // v4l2
      ioctl(device_,VIDIOC_S_CTRL,&ctrl);
      ioctl(device_,VIDIOC_G_CTRL,&ctrl);
      picture_.whiteness=ctrl.value;
   }
}

void QCamV4L2::refreshPictureSettings() {
   v4l2_control ctrl;
   memset(&ctrl,0,sizeof(v4l2_control));
   // get properties
   if (options_ & haveBrightness) {
   // brightness
   ctrl.id=V4L2_CID_BRIGHTNESS;
   // v4l2
   ioctl(device_,VIDIOC_G_CTRL,&ctrl);
   picture_.brightness=ctrl.value;
   }
   if (options_ & haveHue) {
   // hue
   ctrl.id=V4L2_CID_HUE;
   // v4l2
   ioctl(device_,VIDIOC_G_CTRL,&ctrl);
   picture_.hue=ctrl.value;
   }
   if (options_ & haveColor) {
   // saturation
   ctrl.id=V4L2_CID_SATURATION;
   // v4l2
   ioctl(device_,VIDIOC_G_CTRL,&ctrl);
   picture_.colour=ctrl.value;
   }
   if (options_ & haveContrast) {
   // contrast
   ctrl.id=V4L2_CID_CONTRAST;
   // v4l2
   ioctl(device_,VIDIOC_G_CTRL,&ctrl);
   picture_.contrast=ctrl.value;
   }
   if (options_ & haveWhiteness) {
   // whiteness
   ctrl.id=V4L2_CID_WHITENESS;
   // v4l2
   ioctl(device_,VIDIOC_G_CTRL,&ctrl);
   picture_.whiteness=ctrl.value;
   }

   if (options_ & haveBrightness) emit brightnessChange(getBrightness());
   if (options_ & haveContrast) emit contrastChange(getContrast());
   if (options_ & haveHue) emit hueChange(getHue());
   if (options_ & haveColor) emit colorChange(getColor());
   if (options_ & haveWhiteness) emit whitenessChange(getWhiteness());
}

QWidget * QCamV4L2::buildGUI(QWidget * parent) {
   QWidget * remoteCTRL=QCam::buildGUI(parent);

   // source box
   infoBox=new QHBox(sourceGroup);

   // palette and input display
   QWidget* padding1=new QWidget(infoBox);
   QLabel* label1=new QLabel("Input :",infoBox);
   sourceB=new QCamComboBox("source",infoBox,sourceNumber,sourceTable,sourceLabel);
   sourceB->setCurrentText(QString((char*)input.name));
   if(sourceNumber<2)
      sourceB->setEnabled(false);
   connect(sourceB,SIGNAL(change(int)),this,SLOT(setSource(int)));
   QWidget* padding2=new QWidget(infoBox);
   QLabel* label2=new QLabel("Palette :",infoBox);
   paletteB=new QCamComboBox("source",infoBox,paletteNumber,paletteTable,paletteLabel);
   paletteB->setCurrentText(QString(supported_palettes[palette].name));
   if(paletteNumber<2)
      paletteB->setEnabled(false);
   connect(paletteB,SIGNAL(change(int)),this,SLOT(setPalette(int)));
   QWidget* padding3=new QWidget(infoBox);
   // tips
   QToolTip::add(sourceB,"V4L2 input used");
   QToolTip::add(paletteB,"V4L2 palette used");

   QLabel* label3=new QLabel("Mode :",infoBox);
   // frame mode number
   int labelNumber;
   if(supported_palettes[palette].mode==GreyFrame)
      labelNumber=1;
   else
      labelNumber=2;
   // raw mode settings box
   int frameModeTable[]={GreyFrame,YuvFrame};
   const char* frameModeLabel[]={"Grey","Color"};
   // adding frames mode
   frameModeB= new QCamComboBox("frame type",infoBox,labelNumber,frameModeTable,frameModeLabel);
   connect(frameModeB,SIGNAL(change(int)),this,SLOT(setMode(int)));
   QWidget* padding4=new QWidget(infoBox);
   // looking for a saved raw mode
   string keyName("COLOR_MODE_");
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
   VctrlBox=new QVBox(ctrlBox);
   QGridBox * hbox= new QGridBox(VctrlBox,Qt::Vertical,3);
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

void QCamV4L2::setSource(int val) {
   string keyName("SOURCE_");
   keyName+=(char*)v4l2_cap_.card;
   settings.setKey(keyName.c_str(),sourceB->text(val).ascii());
   QMessageBox::information(0,"Qastrocam-g2","Please restart Qastrocam-g2\nto get the new source");
}

// changing palette
void QCamV4L2::setPalette(int val) {
   palette=getSupportedPaletteIndex(paletteB->text(val));
   updatePalette();
   allocBuffers();
   // updating mmap
   if(useMmap)
      useMmap=mmapInit();
   // setting controls back
   updatePictureSettings();
   // gui update
   if(supported_palettes[palette].mode==YuvFrame)
      frameModeB->setEnabled(true);
   else {
      frameModeB->setEnabled(false);
      frameModeB->setCurrentItem(0);
      mode_=GreyFrame;
      inputBuffer_.setMode(mode_);
   }
}

// changing raw mode
void  QCamV4L2::setMode(int  val) {
   QString value=frameModeB->text(val);
   string keyName("COLOR_MODE_");
   keyName+=(char*)v4l2_cap_.card;
   settings.setKey(keyName.c_str(),value.data());
   setMode((ImageMode)val);
}

// changing raw mode
void  QCamV4L2::setMode(ImageMode val) {
   switch (val) {
   case YuvFrame:
      if (supported_palettes[palette].mode != GreyFrame) {
         // color mode possible only if palette is not B&W
         mode_=val;
      }
      break;
   case GreyFrame:
      mode_=val;
      break;
   }
   inputBuffer_.setMode(mode_);
}

// mmap init
bool QCamV4L2::mmapInit() {
   // setting struct
   mmap_reqbuf.count=buffNumber;

   // request buffs
   if(ioctl(device_,VIDIOC_REQBUFS, &mmap_reqbuf)==-1) {
      cout << "Troubles with video streaming, using read/write" << endl;
      mmap_reqbuf.count=-1;
      mmapRelease();
      return(false);
   }
   cout << "Using " << mmap_reqbuf.count << " buffers for video streaming" << endl;

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
         perror("mmap buf");
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
uchar* QCamV4L2::mmapCapture() {
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

void QCamV4L2::mmapRelease() {
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
double QCamV4L2::getTime() {
   double t;
   struct timeval tv;
   gettimeofday(&tv,NULL);
   t=(float)tv.tv_usec/(float)1000000;
   t+=tv.tv_sec;
   return(t);
}
