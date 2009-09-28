#include "QCamV4L.moc"
#include <iostream>
#include <sstream>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <string>
#include <qtabwidget.h>
#include <qsocketnotifier.h>
#include <qtimer.h> 

#include "QCamSlider.hpp"
#include "ccvt.h"
#include "QGridBox.hpp"
#include "QCamComboBox.hpp"
#include "SettingsBackup.hpp"

extern settingsBackup settings;

const int QCamV4L::DefaultOptions=(haveBrightness|haveContrast|haveHue|haveColor|haveWhiteness);

QCamV4L::QCamV4L(const char * devpath,int preferedPalette, const char* devsource,
                 unsigned long options /* cf QCamV4L::options */) {
   v4l2_input input;
   v4l2_std_id _id;
   v4l2_standard standard;
   int _index=0;

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
   window_.x=0;
   window_.y=0;
   window_.clips=NULL;
   window_.clipcount=0;
   devpath_=devpath;

   if (-1 == (device_=open(devpath_.c_str(),
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

   cout << "device name : " << capability_.name << endl;

   /* available inputs */
   cout << endl << "available inputs : " << endl;
   input.index=0;
   while(!ioctl(device_,VIDIOC_ENUMINPUT,&input)) {
      cout << "input #" << input.index << " : " << input.name << endl;
      input.index++;
   }

   /* choosing stored source */
   int res;
   string keyName("SOURCE_");
   keyName+=capability_.name;

   if(strlen(devsource)) settings.setKey(keyName.c_str(),devsource);
   if(settings.haveKey(keyName.c_str())) {
      cout << "found stored source : " << settings.getKey(keyName.c_str()) << endl ;
      string source=settings.getKey(keyName.c_str());
      input.index=0;
      do {
         res=ioctl(device_,VIDIOC_ENUMINPUT,&input);
         input.index++;
      } while((res==0)&&(strcasecmp(source.c_str(),(char*)input.name)!=0));
      if(res==-1)
	cout << "source '" << settings.getKey(keyName.c_str()) << "' not found, using default" << endl;
      else {
         _index=input.index-1;
         ioctl (device_, VIDIOC_S_INPUT, &_index);
      }
   } else cout << "\nIn order to set the default source\nfor this device, use the -i option\n(generic V4L devices only)\n\n" ;

   /* reading input */
   ioctl(device_,VIDIOC_G_INPUT,&_index);
   input.index=_index;
   ioctl(device_,VIDIOC_ENUMINPUT,&input);
   cout << "using : " << input.name << endl << endl; 

   /* storing default source */ 
   settings.setKey(keyName.c_str(),(char*)input.name);

   /* getting video standard */
   if(ioctl(device_,VIDIOC_G_STD,&_id)==-1) {
      perror("Getting Standard"); 
   };
   //cout << _index << endl;

   standard.index=0;
   res=0;
   while((res!=-1)&&(standard.id!=_id)) {
      res=ioctl(device_,VIDIOC_ENUMSTD,&standard);
      standard.index++;
   }
   if(res!=0)
      cout << "unable to get video standard, setting default frame rate : " << frameRate_ << " i/s\n" ;  
   else {
      cout << "Video standard : " << standard.name << endl;
      frameRate_=standard.frameperiod.denominator/standard.frameperiod.numerator;
      cout <<  "Using Framerate : " << frameRate_ << endl;
   }

   cout << "initial size "<<window_.width<<"x"<<window_.height<<"\n";

   mmap_buffer_=NULL;
   if (mmapInit()) {
      mmapCapture();
   }

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

   setProperty("CameraName",capability_.name);
   setProperty("FrameRateSecond",frameRate_);

   label(capability_.name);
}

void QCamV4L::resize(const QSize & s) {
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
	 /* trying VIDEO_PALETTE_RGB24 */
         picture_.palette=VIDEO_PALETTE_RGB24;
         if ( 0== ioctl(device_, VIDIOCSPICT, &picture_)) {
            cout << "found palette VIDEO_PALETTE_RGB24"<<endl;
            break;
         }
         cout <<"VIDEO_PALETTE_RGB24 not supported.\n";
	 /* trying VIDEO_PALETTE_YUYV */
         picture_.palette=VIDEO_PALETTE_YUYV;
         if ( 0== ioctl(device_, VIDIOCSPICT, &picture_)) {
            cout << "found palette VIDEO_PALETTE_YUYV"<<endl;
           break;
         }
         cout <<"VIDEO_PALETTE_YUYV not supported.\n";
	 /* trying VIDEO_PALETTE_YUV420P (Planar) */
         picture_.palette=VIDEO_PALETTE_YUV420P;
         if (0 == ioctl(device_, VIDIOCSPICT, &picture_)) {
            cout << "found palette VIDEO_PALETTE_YUV420P"<<endl;
            break;
         }
         cout <<"VIDEO_PALETTE_YUV420P not supported.\n"; 
         /* trying VIDEO_PALETTE_YUV420 (interlaced) */
         picture_.palette=VIDEO_PALETTE_YUV420;
         if ( 0== ioctl(device_, VIDIOCSPICT, &picture_)) {
            cout << "found palette VIDEO_PALETTE_YUV420"<<endl;
            break;
         }
         cout <<"VIDEO_PALETTE_YUV420 not supported.\n";
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
   
   allocBuffers();
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

   case VIDEO_PALETTE_YUV420:
   case VIDEO_PALETTE_YUV420P:
      tmpBuffer_=new uchar[(int)window_.width * window_.height * 3/2 ];
      break;
   case VIDEO_PALETTE_YUV422:
   case VIDEO_PALETTE_YUYV:
      tmpBuffer_=new uchar[(int)window_.width * window_.height * 2];
      break;
   default:
      tmpBuffer_=NULL;
   }
}

const QSize * QCamV4L::getAllowedSize() const {
   if (sizeTable_==NULL) {
      int currentIndex=0;
      int currentx=capability_.maxwidth;
      int currenty=capability_.maxheight;
      struct video_window testWindow;
      sizeTable_=new QSize[8];

      cout << "Frame size detection" << endl;

      testWindow.x=0;
      testWindow.y=0;

      while((currentIndex<7)&&(currentx>capability_.minwidth)&&(currenty>capability_.minheight)) {
         testWindow.width=currentx;
         testWindow.height=currenty;
         if (ioctl(device_, VIDIOCSWIN, &testWindow)!=-1) {
            if(ioctl(device_, VIDIOCGWIN, &testWindow)!=-1) {
               sizeTable_[currentIndex]=QSize(testWindow.width,testWindow.height);
               currentIndex++;
               cout << "Adding " << testWindow.width << "x" << testWindow.height<< endl;
            }
         }
         currentx/=2;
         currenty/=2;
         sizeTable_[currentIndex]=QSize(0,0);
      }
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
   static char buff[11];

   checkSize(x,y);
   window_.x=0;
   window_.y=0;
   window_.width=x;
   window_.height=y;

   cout << "trying x=" << window_.width
        << " " << "y=" << window_.height <<endl;
   if(ioctl(device_, VIDIOCSWIN, &window_)) {
       perror ("ioctl(VIDIOCSWIN)");
   } 
   if(ioctl(device_, VIDIOCGWIN, &window_)) {
       perror ("ioctl(VIDIOCGWIN)");
   }
   cout << "set to x=" << window_.width
        << " " << "y=" << window_.height <<endl;
   snprintf(buff,10,"%dx%d",window_.width,window_.height);
   setProperty("FrameSize",buff,true);

   if((x!=window_.width)||(y!=window_.height)) {
      cout << "Some devices refuse hot-resizing,\nYou should quit to get the new size" << endl;
   }

   allocBuffers();
   
   return true;
}

bool QCamV4L::dropFrame() {
   static char nullBuff[720*576*4];
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
      case VIDEO_PALETTE_YUV420:
         bufSize=window_.width * window_.height *3/2;
         break;
      case VIDEO_PALETTE_RGB24:
         bufSize=window_.width * window_.height *3;
         break;
      case VIDEO_PALETTE_YUV422:
      case VIDEO_PALETTE_YUYV:
         bufSize=window_.width * window_.height *2;
         break;
      default:
         cerr << "invalid palette "<<picture_.palette<<endl;
         exit(1);
      }
      return 0 < read(device_,(void*)nullBuff,bufSize);
   }
}

bool QCamV4L::updateFrame() {
   static char nullBuf[720*576];
   bool res;
   void * YBuf=NULL,*UBuf=NULL,*VBuf=NULL;
   YBuf=(void*)yuvBuffer_.YforOverwrite();
   switch(mode_) {
   case GreyFrame:
   case RawRgbFrame1:
   case RawRgbFrame2:
   case RawRgbFrame3:
   case RawRgbFrame4:
      UBuf=VBuf=nullBuf;
      break;
   case YuvFrame:
      UBuf=(void*)yuvBuffer_.UforOverwrite();
      VBuf=(void*)yuvBuffer_.VforOverwrite();
   }

   if (mmap_buffer_) {
      mmapCapture();
      mmapSync();
      setTime();
      res=true;
      
      switch (picture_.palette) {
      case VIDEO_PALETTE_GREY:
         memcpy(YBuf,mmapLastFrame(),window_.width * window_.height);
         break;
      case VIDEO_PALETTE_YUV420P:
         memcpy(YBuf,mmapLastFrame(), window_.width * window_.height);
         memcpy(UBuf,
                mmapLastFrame()+ window_.width * window_.height,
                (window_.width/2) * (window_.height/2));
         memcpy(VBuf,
                mmapLastFrame()+ window_.width * window_.height+(window_.width/2) * (window_.height/2),
                (window_.width/2) * (window_.height/2));
         break;
      case VIDEO_PALETTE_YUV420:
         ccvt_420i_420p(window_.width,window_.height,
                           mmapLastFrame(),
                           YBuf,
                           UBuf,
                           VBuf);
         break;
      case VIDEO_PALETTE_RGB24:
         // trouble, rgb24_420p not yet available
         ccvt_rgb24_420p(window_.width,window_.height,
                            mmapLastFrame(),
                         YBuf,
                         UBuf,
                         VBuf);
         break;
      case VIDEO_PALETTE_YUV422:
      case VIDEO_PALETTE_YUYV:
         ccvt_yuyv_420p(window_.width,window_.height,
                         tmpBuffer_,
                         YBuf,
                         UBuf,
                         VBuf);
         break;

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
      res = 0 < read(device_,YBuf,window_.width * window_.height);
      if (res) setTime();
      res = res && (0 < read(device_,UBuf,(window_.width/2) * (window_.height/2)));
      res = res && (0 < read(device_,VBuf,(window_.width/2) * (window_.height/2)));
      break;
   case VIDEO_PALETTE_YUV420:
      res = 0 < read(device_,(void*)tmpBuffer_,window_.width * window_.height *3/2);
      if (res) {
         setTime();
         ccvt_420i_420p(window_.width,window_.height,
                        tmpBuffer_,
                        YBuf,
                        UBuf,
                        VBuf);
      }
      break;
   case VIDEO_PALETTE_RGB24:
      // trouble, rgb24_420p not yet available
      res = 0 < read(device_,(void*)tmpBuffer_,window_.width * window_.height * 3);
      if (res) {
         setTime();
         ccvt_rgb24_420p(window_.width,window_.height,
                         tmpBuffer_,
                         YBuf,
                         UBuf,
                         VBuf);
      }
      break;
   case VIDEO_PALETTE_YUV422:
   case VIDEO_PALETTE_YUYV:
      res = 0 < read(device_,(void*)tmpBuffer_,window_.width * window_.height * 2);
      if (res) {
          setTime();
          ccvt_yuyv_420p(window_.width,window_.height,
                         tmpBuffer_,
                         YBuf,
                         UBuf,
                         VBuf);
      }
      break;
   default:
      cerr << "invalid palette "<<picture_.palette<<endl;
      exit(1);
   }
   }
   if (res) {
      newFrameAvaible();
        if (options_ & haveBrightness) emit brightnessChange(getBrightness());
        if (options_ & haveContrast) emit contrastChange(getContrast());
        if (options_ & haveHue) emit hueChange(getHue());
        if (options_ & haveColor) emit colorChange(getColor());
        if (options_ & haveWhiteness) emit whitenessChange(getWhiteness());
   } else {
      perror("updateFrame");
      cout << "frame dropped" << endl;
      newFrameAvaible();
   }
   int newFrameRate=getFrameRate();
   if (frameRate_ != newFrameRate) {
      frameRate_=newFrameRate;
      if (timer_) timer_->changeInterval(1000/frameRate_);
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

   if(mmap_buffer_!=NULL)
      munmap(mmap_buffer_,mmap_mbuf_.size);

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
   int labelNumber;
   if(picture_.palette==VIDEO_PALETTE_GREY)
      labelNumber=1;
   else
      labelNumber=6;

   int frameModeTable[]={GreyFrame,YuvFrame,RawRgbFrame1,RawRgbFrame2,RawRgbFrame3,RawRgbFrame4};
   const char* frameModeLabel[]={"Grey", "RGB", "Raw color GR","Raw color RG (Vesta)","Raw color BG (TUC)","Raw color GB"};
   frameModeB= new QCamComboBox("frame type",remoteCTRL,labelNumber,frameModeTable,frameModeLabel);
   connect(frameModeB,SIGNAL(change(int)),this,SLOT(setMode(int)));

   if(settings.haveKey("RAW_MODE")) {
	int index=frameModeB->getPosition(settings.getKey("RAW_MODE"));
	if (index!=-1) setMode(index);
	frameModeB->setCurrentItem(index);
   } else setMode(0);

   if (options_ & haveContrast) {
      remoteCTRLcontrast_=new QCamSlider("Cont.",false,hbox);
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

   stringstream sb;
   sb << frameRate_;
   remoteCTRLlx= new QHGroupBox(tr("long exposure"),remoteCTRL);
   lxLabel1= new QLabel("fps :",remoteCTRLlx);
   lxRate= new QLabel(sb.str(),remoteCTRLlx);
   lxRate->setAlignment(AlignLeft|AlignVCenter);
   lxRate->setMinimumWidth(32);
   int lxTable[]={lxNone,lxPar,lxSer};
   const char* lxLabel[]={"lx : none","lx : // port","lx : serial"};   
   lxSelector=new QCamComboBox(tr("lxMode"),remoteCTRLlx,3,lxTable,lxLabel);
   lxLabel2=new QLabel("Delay (s) :",remoteCTRLlx);
   lxTime=new QLineEdit(remoteCTRLlx);
   lxTime->setMaximumWidth(48);
   lxTime->setEnabled(false);
   lxBar=new QProgressBar(remoteCTRLlx);

   // temp, not finished
   remoteCTRLlx->hide();
   //

   return remoteCTRL;
}

void  QCamV4L::setMode(int  val) {
   QString value=frameModeB->text(val);
   settings.setKey("RAW_MODE",value.data());
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
   case RawRgbFrame1:
   case RawRgbFrame2:
   case RawRgbFrame3:
   case RawRgbFrame4:
      mode_=val;
      break;
   }
   yuvBuffer_.setMode(mode_);
}

bool QCamV4L::mmapInit() {
   struct v4l2_requestbuffers stream_buffers;

   mmap_mbuf_.size = 0;
   mmap_mbuf_.frames = 0;
   mmap_last_sync_buff_=-1;
   mmap_last_capture_buff_=-1;
   mmap_buffer_=NULL;

   if (ioctl(device_, VIDIOCGMBUF, &mmap_mbuf_)) {
      cout << "mmap not supported" << endl;
      return false;
   }

   mmap_buffer_=(uchar *)mmap(NULL, mmap_mbuf_.size, PROT_READ, MAP_SHARED, device_, 0);
   if (mmap_buffer_ == MAP_FAILED) {
      perror("mmap");
      mmap_mbuf_.size = 0;
      mmap_mbuf_.frames = 0;
      mmap_buffer_=NULL;
      cout << "Trouble with mmap, using read/write mode" << endl;
      return false;
   }
   cout << "mmap() in use : "
        << "frames="<<mmap_mbuf_.frames
        <<" size="<<mmap_mbuf_.size
        << endl;

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
   //if (mmap_curr_buff_ == 1 ) {
   //   return mmap_buffer_;
   //} else {
   //   return mmap_buffer_ + mmap_mbuf_.offsets[1];
   //}
   //return mmap_buffer_ + mmap_mbuf_.offsets[(mmap_curr_buff_-1)% mmap_mbuf_.frames];
   //return mmap_buffer_ + mmap_mbuf_.size*((mmap_curr_buff_-1)%mmap_mbuf_.frames);
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
