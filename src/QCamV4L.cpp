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

/******************************/
/* only available for kernel2 */
/******************************/


#if KERNEL_2

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>

#include <iostream>

#include <Qt/qtabwidget.h>
#include <Qt/qsocketnotifier.h>
#include <Qt/qtimer.h>
#include <Qt/qtooltip.h>

#include "yuv.hpp"
#include "QCamV4L.hpp"
#include "QCamSlider.hpp"
#include "QGridBox.hpp"
#include "QCamComboBox.hpp"

const int QCamV4L::DefaultOptions=(haveBrightness|haveContrast|haveHue|haveColor|haveWhiteness);

QCamV4L::QCamV4L(const char * devpath,int preferedPalette,
                 unsigned long options /* cf QCamV4L::options */) {
   options_=options;
   tmpBuffer_=NULL;
   remoteCTRLbrightness_=NULL;
   remoteCTRLcontrast_=NULL;
   remoteCTRLhue_=NULL;
   remoteCTRLcolor_=NULL;
   remoteCTRLwhiteness_=NULL;
   sizeTable_=NULL;
   frameRate_=10;
   device_=-1;
   if (-1 == (device_=open(devpath,
                           O_RDONLY | ((options_ & ioNoBlock)?O_NONBLOCK:0)))) {
      perror(devpath);
   }
   if (device_ != -1) {
      if (-1 == ioctl(device_,VIDIOCGCAP,&capability_)) {
         perror ("ioctl (VIDIOCGCAP)");
      }
      if (-1 == ioctl (device_, VIDIOCGWIN, &window_)) {
         perror ("ioctl (VIDIOCGWIN)");
      }
      if (-1 == ioctl (device_, VIDIOCGPICT, &picture_)) {
         perror ("ioctl (VIDIOCGPICT)");
      }
      init(preferedPalette);
   }

#if 1
   cout << "initial size "<<window_.width<<"x"<<window_.height<<"\n";
#endif
   notifier_=NULL;
   timer_=NULL;
   if (options_&ioUseSelect) {
      notifier_ = new QSocketNotifier(device_, QSocketNotifier::Read, this);
      connect(notifier_,SIGNAL(activated(int)),this,SLOT(updateFrame()));
      cout << "Using select to wait new frames.\n";
   } else {
      timer_=new QTimer(this);
      connect(timer_,SIGNAL(timeout()),this,SLOT(updateFrame()));
      timer_->start(1000/frameRate_) ; // value 0 => called every time event loop is empty
      cout << "Using timer to wait new frames.\n";
   }
   mmap_buffer_=NULL;
   if (mmapInit()) {
      mmapCapture();
   }
   label(capability_.name);

   sizeTable=getAllowedSize();
}

void QCamV4L::resize(const QSize & s) {
#if 1
   cout << "QCamV4L::resize("
        << s.width()
        << "x"
        << s.height()
        << ")"
        << endl;
#endif
   setSize(s.width(),s.height());
}

void QCamV4L::init(int preferedPalette) {
   // setting palette
   if (preferedPalette) {
      picture_.palette=preferedPalette;
      if (0 == ioctl(device_, VIDIOCSPICT, &picture_)) {
         cout << "found preferedPalette "
              << preferedPalette << endl;
      } else {
         preferedPalette=0;
         cout << "preferedPalette "
              << preferedPalette
              << " invalid, trying to find one."<< endl;
      }
   }
   if (preferedPalette == 0) {
      do {
         /* trying VIDEO_PALETTE_YUV420P (Planar) */
         picture_.palette=VIDEO_PALETTE_YUV420P;
         if (0 == ioctl(device_, VIDIOCSPICT, &picture_)) {
            cout << "found palette VIDEO_PALETTE_YUV420P"<<endl;
            break;
         }
         cout <<"VIDEO_PALETTE_YUV420P not supported.\n";
         /* trying VIDEO_PALETTE_RGB24 */
         picture_.palette=VIDEO_PALETTE_RGB24;
         if ( 0== ioctl(device_, VIDIOCSPICT, &picture_)) {
            cout << "found palette VIDEO_PALETTE_RGB24"<<endl;
            break;
         }
         cout <<"VIDEO_PALETTE_RGB24 not supported.\n";
         /* trying VIDEO_PALETTE_GREY */
         picture_.palette=VIDEO_PALETTE_GREY;
         if ( 0== ioctl(device_, VIDIOCSPICT, &picture_)) {
            cout << "found palette VIDEO_PALETTE_GREY"<<endl;
            break;
         }
         cout <<"VIDEO_PALETTE_GREY not supported.\n";
         cerr <<"could not find a supported palette.\n";
         exit(1);
      }
      while (false);
   }
   mode_ = (picture_.palette==VIDEO_PALETTE_GREY)?GreyFrame:YuvFrame;
   if (ioctl(device_, VIDIOCGPICT, &picture_)) {
      perror ("ioctl(VIDIOCGPICT)");
   }
   allocBuffers();

   setProperty("CameraName",capability_.name);
}

void QCamV4L::allocBuffers() {
   delete tmpBuffer_;
   yuvBuffer_.setSize(QSize(window_.width,window_.height));
   switch (picture_.palette) {
   case VIDEO_PALETTE_GREY:
      tmpBuffer_=new uchar[(int)window_.width * window_.height];
      break;
   case VIDEO_PALETTE_RGB24:
      tmpBuffer_=new uchar[(int)window_.width * window_.height * 3];
      break;
   case VIDEO_PALETTE_YUV420P:
      tmpBuffer_=new uchar[(int)window_.width * window_.height * 3/2 ];
      break;
#if 0
   case VIDEO_PALETTE_YUV422:
   case VIDEO_PALETTE_YUYV:
      tmpBuffer_=new uchar[(int)window_.width * window_.height * 2];
      break;
#endif
   default:
      tmpBuffer_=NULL;
   }
}

const QSize * QCamV4L::getAllowedSize() const {
   if (sizeTable_==NULL) {
      QSize min=QSize(capability_.minwidth,capability_.minheight);
      QSize current=size();
      QSize max=QSize(capability_.maxwidth,capability_.maxheight);
      int currentIndex=0;
      sizeTable_=new QSize[4];
      sizeTable_[currentIndex++]=min;
      if (min!=current) {
         sizeTable_[currentIndex++]=current;
      }
      if (max!=current) {
         sizeTable_[currentIndex++]=max;
      }
      sizeTable_[currentIndex]=QSize(0,0);
   }
   return sizeTable_;
}

void QCamV4L::checkSize(int & x, int & y) const {
   if (x>=capability_.maxwidth && y >= capability_.maxheight) {
      x=capability_.maxwidth;
      y=capability_.maxheight;
   } else if (x<=capability_.minwidth || y <=capability_.minheight ) {
      x=capability_.minwidth;
      y=capability_.minheight;
   }
}

bool QCamV4L::setSize(int x, int y) {
   checkSize(x,y);
   window_.width=x;
   window_.height=y;

   static char buff[11];
   snprintf(buff,10,"%dx%d",x,y);
   setProperty("FrameSize",buff,true);
#if 1
   cout << "x=" << window_.width
        << " " << "y=" << window_.height <<endl;
#endif
   if (ioctl (device_, VIDIOCSWIN, &window_)) {
       perror ("ioctl(VIDIOCSWIN)");
   }
   ioctl (device_, VIDIOCGWIN, &window_);

   allocBuffers();
   return true;
}

bool QCamV4L::dropFrame() {
   static char nullBuff[640*480*4];
   int bufSize;
   if (mmap_buffer_) {
      mmapCapture();
      mmapSync();
      return true;
   } else {
      switch (picture_.palette) {
      case VIDEO_PALETTE_GREY:
         bufSize=window_.width * window_.height;
         break;
      case VIDEO_PALETTE_YUV420P:
         bufSize=window_.width * window_.height *3/2;
         break;
      case VIDEO_PALETTE_RGB24:
         bufSize=window_.width * window_.height *3;
         break;
      default:
         cerr << "invalid palette "<<picture_.palette<<endl;
         exit(1);
      }
      return 0 < read(device_,(void*)nullBuff,bufSize);
   }
}

bool QCamV4L::updateFrame() {
   static char nullBuf[640*480];
   bool res;
   void * YBuf=NULL,*UBuf=NULL,*VBuf=NULL;
   //cout <<"O"<<flush;
   YBuf=(void*)yuvBuffer_.YforOverwrite();
   if(mode_==YuvFrame) {
      UBuf=(void*)yuvBuffer_.UforOverwrite();
      VBuf=(void*)yuvBuffer_.VforOverwrite();
   } else {
      UBuf=nullBuf;
      VBuf=nullBuf;
   }

   if (mmap_buffer_) {
      //cout <<"c"<<flush;
      mmapCapture();

      //cout <<"s"<<flush;
      mmapSync();
      setTime();
      res=true;

      switch (picture_.palette) {
      case VIDEO_PALETTE_GREY:
         memcpy(YBuf,mmapLastFrame(),window_.width * window_.height);
         break;
      case VIDEO_PALETTE_YUV420P:
         /*memcpy(YBuf,mmapLastFrame(), window_.width * window_.height);
         memcpy(UBuf,
                mmapLastFrame()+ window_.width * window_.height,
                (window_.width/2) * (window_.height/2));
         memcpy(VBuf,
                mmapLastFrame()+ window_.width * window_.height+(window_.width/2) * (window_.height/2),
                (window_.width/2) * (window_.height/2));*/
         yuv420_to_yuv444(window_.width,window_.height,(unsigned char*)mmapLastFrame(),
            (unsigned char*)mmapLastFrame()+ window_.width * window_.height,
            (unsigned char*)mmapLastFrame()+ window_.width * window_.height+(window_.width/2) * (window_.height/2),
            (unsigned char*)YBuf,(unsigned char*)UBuf,(unsigned char*)VBuf);
         break;
#if 1
      case VIDEO_PALETTE_RGB24:
         rgb24_to_yuv444(window_.width,window_.height,
                            (unsigned char*)mmapLastFrame(),
                         (unsigned char*)YBuf,
                         (unsigned char*)UBuf,
                         (unsigned char*)VBuf);
         break;
#endif

      default:
         cerr << "invalid palette "<<picture_.palette<<endl;
         exit(1);
      }
   } else {
   switch (picture_.palette) {
   case VIDEO_PALETTE_GREY:
      res = 0 < read(device_,YBuf,window_.width * window_.height);
      if (res) setTime();
      break;
   case VIDEO_PALETTE_YUV420P:
      unsigned char* temp_buffer;
      temp_buffer=(unsigned char*)malloc(window_.width * window_.height*2/3);
      res = 0 < read(device_,temp_buffer,window_.width * window_.height*2/3);
      if (res) setTime();
      yuv420_to_yuv444(window_.width,window_.height,temp_buffer,
      (unsigned char*)temp_buffer+ window_.width * window_.height,
      (unsigned char*)temp_buffer+ window_.width * window_.height+(window_.width/2) * (window_.height/2),
      (unsigned char*)YBuf,(unsigned char*)UBuf,(unsigned char*)VBuf);
      free(temp_buffer);
      break;
#if 1
   case VIDEO_PALETTE_RGB24:
      res = 0 < read(device_,(void*)tmpBuffer_,window_.width * window_.height * 3);
      if (res) {
         setTime();
         rgb24_to_yuv444(window_.width,window_.height,
                         tmpBuffer_,
                         (unsigned char*)YBuf,
                         (unsigned char*)UBuf,
                         (unsigned char*)VBuf);
      }
      break;
#endif
   default:
      cerr << "invalid palette "<<picture_.palette<<endl;
      exit(1);
   }
   }
   if (res) {
      newFrameAvaible();
      //cout <<"b"<<flush;
      /*
        if (options_ & haveBrightness) emit brightnessChange(getBrightness());
        if (options_ & haveContrast) emit contrastChange(getContrast());
        if (options_ & haveHue) emit hueChange(getHue());
        if (options_ & haveColor) emit colorChange(getColor());
        if (options_ & haveWhiteness) emit whitenessChange(getWhiteness());
      */
      //cerr << "+";
   } else {
      //newFrameAvaible();
      //perror("updateFrame");
      //cerr << ".";
   }
   //cout <<"k"<<flush;
   int newFrameRate=getFrameRate();
   if (frameRate_ != newFrameRate) {
      frameRate_=newFrameRate;
      if (timer_)
         timer_->setInterval(1000/frameRate_);
   }
   return res;
}

const QSize & QCamV4L::size() const {
   return yuvBuffer_.size();
}


void QCamV4L::setContrast(int val) {
   picture_.contrast=val;
   updatePictureSettings();
}

int QCamV4L::getContrast() const {
   return picture_.contrast;
}

void QCamV4L::setBrightness(int val) {
   picture_.brightness=val;
   updatePictureSettings();
}

int QCamV4L::getBrightness() const {
   return picture_.brightness;
}

void QCamV4L::setColor(int val) {
   picture_.colour=val;
   updatePictureSettings();
}

int QCamV4L::getColor() const {
   return picture_.colour;
}

void QCamV4L::setHue(int val) {
   picture_.hue=val;
   updatePictureSettings();
}

int QCamV4L::getHue() const {
   return picture_.hue;
}

void QCamV4L::setWhiteness(int val) {
   picture_.whiteness=val;
   updatePictureSettings();
}

int QCamV4L::getWhiteness() const {
   return picture_.whiteness;
}

QCamV4L::~QCamV4L() {
   delete tmpBuffer_;
   close(device_);
}

void QCamV4L::updatePictureSettings() {
   if (ioctl(device_, VIDIOCSPICT, &picture_) ) {
      perror("updatePictureSettings");
   }
   ioctl(device_, VIDIOCGPICT, &picture_);
}

void QCamV4L::refreshPictureSettings() {
   if (ioctl(device_, VIDIOCGPICT, &picture_) ) {
      perror("refreshPictureSettings");
   }
   if (options_ & haveBrightness) emit brightnessChange(getBrightness());
   if (options_ & haveContrast) emit contrastChange(getContrast());
   if (options_ & haveHue) emit hueChange(getHue());
   if (options_ & haveColor) emit colorChange(getColor());
   if (options_ & haveWhiteness) emit whitenessChange(getWhiteness());
}


QWidget * QCamV4L::buildGUI(QWidget * parent) {
   QWidget * remoteCTRL=QCam::buildGUI(parent);
   //QHBox * hbox=new QHBox(remoteCTRL);
   QGridBox * hbox= new QGridBox(remoteCTRL,Qt::Vertical,3);
   /*
     QCheckBox * greyModeB = new QCheckBox("B&W",hbox);
     connect(greyModeB,SIGNAL(toggled(bool)),this,SLOT(setGrey(bool)));
   */

   cropLabel->hide();
   cropCombo->hide();

   int frameModeTable[]={YuvFrame,GreyFrame};
   const char* frameModeLabel[]={"RGB","Grey"};
   QCamComboBox * frameModeB= new QCamComboBox("frame type",remoteCTRL,2,frameModeTable,frameModeLabel);
   frameModeB->setToolTip(tr("Frame type"));
   connect(frameModeB,SIGNAL(change(int)),
           this,SLOT(setMode(int)));
   if (options_ & haveContrast) {
      remoteCTRLcontrast_=new QCamSlider("Cont",false,hbox);
      //hbox->add(remoteCTRLcontrast_);
      connect(this,SIGNAL(contrastChange(int)),
              remoteCTRLcontrast_,SLOT(setValue(int)));
      connect(remoteCTRLcontrast_,SIGNAL(valueChange(int)),
              this,SLOT(setContrast(int)));
   }
   if (options_ & haveBrightness) {
      remoteCTRLbrightness_=new QCamSlider("Bri.",false,hbox);
      //hbox->add(remoteCTRLbrightness_);
      connect(this,SIGNAL(brightnessChange(int)),
              remoteCTRLbrightness_,SLOT(setValue(int)));
      connect(remoteCTRLbrightness_,SIGNAL(valueChange(int)),
              this,SLOT(setBrightness(int)));
   }
   if (options_ & haveHue) {
      remoteCTRLhue_=new QCamSlider("Hue",false,hbox);
      //hbox->add(remoteCTRLhue_);
      connect(this,SIGNAL(hueChange(int)),
              remoteCTRLhue_,SLOT(setValue(int)));
      connect(remoteCTRLhue_,SIGNAL(valueChange(int)),
              this,SLOT(setHue(int)));
   }
   if (options_ & haveColor) {
      remoteCTRLcolor_=new QCamSlider("Col.",false,hbox);
      connect(this,SIGNAL(colorChange(int)),
              remoteCTRLcolor_,SLOT(setValue(int)));
      connect(remoteCTRLcolor_,SIGNAL(valueChange(int)),
              this,SLOT(setColor(int)));
   }
   if (options_ & haveWhiteness) {
      remoteCTRLwhiteness_=new QCamSlider("Whit.",false,hbox);
      connect(this,SIGNAL(whitenessChange(int)),
              remoteCTRLwhiteness_,SLOT(setValue(int)));
      connect(remoteCTRLwhiteness_,SIGNAL(valueChange(int)),
              this,SLOT(setWhiteness(int)));
   }
   //greyModeB->show();
   //remoteCTRLcontrast_->show();
   //remoteCTRLbrightness_->show();
   return remoteCTRL;
}

void  QCamV4L::setMode(int  val) {
   setMode((ImageMode)val);
}

void  QCamV4L::setMode(ImageMode val) {
   switch (val) {
   case YuvFrame:
      if (picture_.palette != VIDEO_PALETTE_GREY) {
         // color mode possible only if palette is not B&W
         mode_=val;
      }
      break;
   case GreyFrame:
      mode_=val;
      break;
   }
   yuvBuffer_.setMode(mode_);
}

bool QCamV4L::mmapInit() {
   mmap_mbuf_.size = 0;
   mmap_mbuf_.frames = 0;
   mmap_last_sync_buff_=-1;
   mmap_last_capture_buff_=-1;
   mmap_buffer_=NULL;

   if (ioctl(device_, VIDIOCGMBUF, &mmap_mbuf_)) {
      // mmap not supported
      return false;
   }
   mmap_buffer_=(uchar *)mmap(NULL, mmap_mbuf_.size, PROT_READ, MAP_SHARED, device_, 0);
   if (mmap_buffer_ == MAP_FAILED) {
      perror("mmap");
      mmap_mbuf_.size = 0;
      mmap_mbuf_.frames = 0;
      mmap_buffer_=NULL;
      return false;
   }
   cout << "mmap() in use: "
        << "frames="<<mmap_mbuf_.frames
      //<<" size="<<mmap_mbuf_.size
        <<"\n";
   /*
   for(int i=0;i<mmap_mbuf_.frames;++i) {
      cout << i<<"="<<mmap_mbuf_.offsets[i]<<"  ";
   }
   */
   cout <<"\n";
   return true;
}

void QCamV4L::mmapSync() {
   mmap_last_sync_buff_=(mmap_last_sync_buff_+1)%mmap_mbuf_.frames;
   if (ioctl(device_, VIDIOCSYNC, &mmap_last_sync_buff_) < 0) {
      perror("QCamV4L::mmapSync()");
   }
}

uchar * QCamV4L::mmapLastFrame() const {
   return mmap_buffer_ + mmap_mbuf_.offsets[mmap_last_sync_buff_];
#if 0
   if (mmap_curr_buff_ == 1 ) {
      return mmap_buffer_;
   } else {
      return mmap_buffer_ + mmap_mbuf_.offsets[1];
   }
   return mmap_buffer_ + mmap_mbuf_.offsets[(mmap_curr_buff_-1)% mmap_mbuf_.frames];
   //return mmap_buffer_ + mmap_mbuf_.size*((mmap_curr_buff_-1)%mmap_mbuf_.frames);
#endif
}

void QCamV4L::mmapCapture() {
   struct video_mmap vm;
   mmap_last_capture_buff_=(mmap_last_capture_buff_+1)%mmap_mbuf_.frames;
   vm.frame = mmap_last_capture_buff_;
   vm.format = picture_.palette;
   vm.width = window_.width;
   vm.height = window_.height;
   if (ioctl(device_, VIDIOCMCAPTURE, &vm) < 0) {
      perror("QCamV4L::mmapCapture");
      // AEW: try and do something sensible here - the V4L source
      // has gone away
      close(device_);
      exit(0);
   }
}

#endif /* KERNEL_2 */
