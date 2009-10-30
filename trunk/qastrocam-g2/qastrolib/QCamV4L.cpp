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
   // opening the video device (non block)
   if (-1 == (device_=open(devpath_.c_str(),
                           O_RDONLY | ((options_ & ioNoBlock)?O_NONBLOCK:0)))) {
      perror(devpath);
   }
   // read device informations
   if (device_ != -1) {
      // v4l2 query cap
      if (-1 == ioctl(device_,VIDIOC_QUERYCAP,&v4l2_cap_)) {
         perror ("ioctl (VIDIOC_QUERYCAP)");
      }
      // v4l
      if (-1 == ioctl (device_, VIDIOCGPICT, &picture_)) {
         perror ("ioctl (VIDIOCGPICT)");
      }
      init(preferedPalette);
   }
   cout << "device name : " << v4l2_cap_.card << endl;
   // enumerate available inputs
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

   // getting video standard to compute frame rate
   // v4l2
   if(ioctl(device_,VIDIOC_G_STD,&_id)==-1) {
      perror("Getting Standard");
   };
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
      frameRate_=10;
      cout << "unable to get video standard, setting default frame rate : " << frameRate_ << " i/s\n" ;
   } else {
      // standard found, computing framerate
      cout << "Video standard : " << standard.name << endl;
      frameRate_=standard.frameperiod.denominator/standard.frameperiod.numerator;
      cout <<  "Using Framerate : " << frameRate_ << endl;
   }

   // mmap init
   mmap_buffer_=NULL;
   if (mmapInit()) {
      mmapCapture();
   }
   // if mmap is NULL, using read/write mode

   // some lx widgets init to avoid segfaults in updateFrame
   lxBar=NULL;
   lxBlink=NULL;

   // setting up the timers
   notifier_=NULL;
   timer_=NULL;
   if (options_&ioUseSelect) {
      // notifier used if the device supports "select"
      notifier_ = new QSocketNotifier(device_, QSocketNotifier::Read, this);
      connect(notifier_,SIGNAL(activated(int)),this,SLOT(updateFrame()));
      cout << "Using select to wait new frames.\n" << endl;
   } else {
      // use a QT timer
      timer_=new QTimer(this);
      connect(timer_,SIGNAL(timeout()),this,SLOT(updateFrame()));
      timer_->start(1000/frameRate_) ; // value 0 => called every time event loop is empty
      cout << "Using timer to wait new frames.\n" << endl;
   }

   // update video stream properties
   setProperty("CameraName",(char*)v4l2_cap_.card);
   setProperty("FrameRateSecond",frameRate_);
   label((char*)v4l2_cap_.card);

   // lx mode vars inits
   //
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
   // setting default settings (we just test palettes)
   v4l2_fmt_.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
   v4l2_fmt_.fmt.pix.width=160;
   v4l2_fmt_.fmt.pix.height=120;
   v4l2_fmt_.fmt.pix.field = V4L2_FIELD_ANY;
   // setting prefered palette if we have one
   // also used for forced palette (-p option)
   if (preferedPalette) {
      v4l2_fmt_.fmt.pix.pixelformat=preferedPalette;
      // v4l2
      /*if (0 == */ioctl(device_, VIDIOC_S_FMT, &v4l2_fmt_)/*) {*/ /**/ ; /**/
         palette="prefered";
         cout << "found preferedPalette " << endl;
         if(v4l2_fmt_.fmt.pix.pixelformat==V4L2_PIX_FMT_GREY)
            mode_=GreyFrame;
         allocBuffers();
         return;
      /*}
      cout << "preferedPalette " << " invalid, trying to find one."<< endl;*/
      // tests removed, due to pwc v4l2 VIDIOC_S_FMT bug

   }
   // else finding a valid palette
   // in high to low quality order
   /* trying VIDEO_PALETTE_RGB24 */
   v4l2_fmt_.fmt.pix.pixelformat=V4L2_PIX_FMT_RGB24;
   // v4l2
   if ( 0== ioctl(device_, VIDIOC_S_FMT, &v4l2_fmt_)) {
      palette="rgb24";
      cout << "found palette VIDEO_PALETTE_RGB24"<<endl;
      allocBuffers();
      return;
   }
   cout <<"VIDEO_PALETTE_RGB24 not supported.\n";
   /* trying VIDEO_PALETTE_YUYV */
   v4l2_fmt_.fmt.pix.pixelformat=V4L2_PIX_FMT_YUYV;
   // v4l2
   if ( 0== ioctl(device_, VIDIOC_S_FMT, &v4l2_fmt_)) {
     palette="yuyv";
     cout << "found palette VIDEO_PALETTE_YUYV"<<endl;
     allocBuffers();
     return;
   }
   cout <<"VIDEO_PALETTE_YUYV not supported.\n";
   /* trying VIDEO_PALETTE_YUV420P (Planar) */
   v4l2_fmt_.fmt.pix.pixelformat=V4L2_PIX_FMT_YUV420;
   // v4l2
   if (0 == ioctl(device_, VIDIOC_S_FMT, &v4l2_fmt_)) {
      palette="yuv420p";
      cout << "found palette VIDEO_PALETTE_YUV420P"<<endl;
      allocBuffers();
      return;
   }
   cout <<"VIDEO_PALETTE_YUV420P not supported.\n";
   /* trying VIDEO_PALETTE_GREY */
   v4l2_fmt_.fmt.pix.pixelformat=V4L2_PIX_FMT_GREY;
   // v4l2
   if ( 0== ioctl(device_, VIDIOC_S_FMT, &v4l2_fmt_)) {
      palette="grey";
      cout << "found palette VIDEO_PALETTE_GREY"<<endl;
      mode_=GreyFrame;
      allocBuffers();
      return;
   }
   cout <<"VIDEO_PALETTE_GREY not supported.\n";
   // no supported palette, leaving
   cerr <<"could not find a supported palette.\n";
   exit(1);
}

// allocate memory for buffers, depending on
// frame size and palette
void QCamV4L::allocBuffers() {
   delete tmpBuffer_;
   yuvBuffer_.setSize(QSize(v4l2_fmt_.fmt.pix.width,v4l2_fmt_.fmt.pix.height));
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
      v4l2_fmt_temp.fmt.pix.field = V4L2_FIELD_ANY;

      // trying small size to get min size
      v4l2_fmt_temp.fmt.pix.width=1;
      v4l2_fmt_temp.fmt.pix.height=1;
      // v4l2
      if (-1 == ioctl(device_,VIDIOC_TRY_FMT,&v4l2_fmt_temp))
         // VIDIOC_TRY_FMT not supported
         if (-1 == ioctl(device_,VIDIOC_S_FMT,&v4l2_fmt_temp))
            perror ("ioctl (VIDIOC_S_FMT)");
      min_x=v4l2_fmt_temp.fmt.pix.width;
      min_y=v4l2_fmt_temp.fmt.pix.height;
      // trying huge size to get max size
      v4l2_fmt_temp.fmt.pix.width=8192;
      v4l2_fmt_temp.fmt.pix.height=8192;
      // v4l2
      if (-1 == ioctl(device_,VIDIOC_TRY_FMT,&v4l2_fmt_temp))
         // VIDIOC_TRY_FMT not supported
         if (-1 == ioctl(device_,VIDIOC_S_FMT,&v4l2_fmt_temp))
            perror ("ioctl (VIDIOC_S_FMT)");
      // most of time, v4l generic supports continous 4 or 8 multiple pixel sizes
      // it gives to much diffrent sizes. We test from max size to min size half by
      // half.
      while((currentIndex<7)&&(v4l2_fmt_temp.fmt.pix.width>min_x)&&(v4l2_fmt_temp.fmt.pix.height>min_y)) {
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
   }
   cout << endl;
   return sizeTable_;
}

// change the frame size
bool QCamV4L::setSize(int x, int y) {
   static char buff[11];

   v4l2_fmt_.fmt.pix.width=x;
   v4l2_fmt_.fmt.pix.height=y;

   // trying the size
   cout << "resizing : x=" << x << " " << "y=" << y <<endl;
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
      v4l2_fmt_.fmt.pix.width=x;
      v4l2_fmt_.fmt.pix.height=y;
      // v4l2
      ioctl(device_, VIDIOC_S_FMT, &v4l2_fmt_);
      // setting mmap back
      if(mmap_buffer_!=NULL) {
         mmap_mbuf_.size = 0;
         mmap_mbuf_.frames = 0;
         mmap_last_sync_buff_=-1;
         mmap_last_capture_buff_=-1;
         // v4l
         ioctl(device_, VIDIOCGMBUF, &mmap_mbuf_);
         mmap_buffer_=(uchar *)mmap(NULL, mmap_mbuf_.size, PROT_READ, MAP_SHARED, device_, 0);
      }
   }
   // updating video stream properties
   snprintf(buff,10,"%dx%d",x,y);
   setProperty("FrameSize",buff,true);
   // realloc buffers using new size
   allocBuffers();
   return(true);
}

// drop frames without treatment
bool QCamV4L::dropFrame() {
   static char* nullBuff=NULL;
   // allocates memory
   if (nullBuff==NULL)
      nullBuff=(char*)malloc(yuvFrameMemSize);
   // mmap case
   if (mmap_buffer_) {
      mmapCapture();
      mmapSync();
      return true;
   }
   // else, read the frame
   return 0 < read(device_,(void*)nullBuff,yuvFrameMemSize);
}

// we should have a new frame
bool QCamV4L::updateFrame() {
   static char nullBuf[720*576];
   bool res;
   double currentTime;
   void * YBuf=NULL,*UBuf=NULL,*VBuf=NULL;
   YBuf=(void*)yuvBuffer_.YforOverwrite();
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
         UBuf=(void*)yuvBuffer_.UforOverwrite();
         VBuf=(void*)yuvBuffer_.VforOverwrite();
   }

   // if we are using mmap
   if (mmap_buffer_) {
      mmapCapture();
      mmapSync();
      res=true;
      tmpBuffer_=mmapLastFrame();
   } else {
   // else we read the device
      res= 0 < read(device_,(void*)tmpBuffer_,yuvFrameMemSize);
      if(!res) perror("FrameUpdate");
   }
   // if we have a frame...
   if(res) {
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
   }
   // if mmap, restoring tmpBuffer_
   if(mmap_buffer_)
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
         if(!yuvBuffer_.isValide(lxLevel)) {
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
   // release buffer mem if needed
   if(tmpBuffer_!=NULL)
      delete tmpBuffer_;
   // release mmap zone i needed
   if(mmap_buffer_!=NULL)
      munmap(mmap_buffer_,mmap_mbuf_.size);
   // close the video device
   close(device_);
}

void QCamV4L::updatePictureSettings() {
   // v4l
   if (ioctl(device_, VIDIOCSPICT, &picture_) ) {
      perror("updatePictureSettings");
   }
   // v4l
   ioctl(device_, VIDIOCGPICT, &picture_);
}

void QCamV4L::refreshPictureSettings() {
   // v4l
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
   QGridBox * hbox= new QGridBox(remoteCTRL,Qt::Vertical,3);

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
      connect(this,SIGNAL(contrastChange(int)),
              remoteCTRLcontrast_,SLOT(setValue(int)));
      connect(remoteCTRLcontrast_,SIGNAL(valueChange(int)),
              this,SLOT(setContrast(int)));
   }
   if (options_ & haveBrightness) {
      remoteCTRLbrightness_=new QCamSlider("Bri.",false,hbox);
      connect(this,SIGNAL(brightnessChange(int)),
              remoteCTRLbrightness_,SLOT(setValue(int)));
      connect(remoteCTRLbrightness_,SIGNAL(valueChange(int)),
              this,SLOT(setBrightness(int)));
   }
   if (options_ & haveHue) {
      remoteCTRLhue_=new QCamSlider("Hue",false,hbox);
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
   yuvBuffer_.setMode(mode_);
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
   mmap_mbuf_.size = 0;
   mmap_mbuf_.frames = 0;
   mmap_last_sync_buff_=-1;
   mmap_last_capture_buff_=-1;
   mmap_buffer_=NULL;

   // v4l
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

// mmap sync
void QCamV4L::mmapSync() {
   mmap_last_sync_buff_=(mmap_last_sync_buff_+1)%mmap_mbuf_.frames;
   // v4l
   if (ioctl(device_, VIDIOCSYNC, &mmap_last_sync_buff_) < 0) {
      perror("QCamV4L::mmapSync()");
   }
}

uchar * QCamV4L::mmapLastFrame() const {
   return mmap_buffer_ + mmap_mbuf_.offsets[mmap_last_sync_buff_];
}

// mmap capture
void QCamV4L::mmapCapture() {
   struct video_mmap vm;
   mmap_last_capture_buff_=(mmap_last_capture_buff_+1)%mmap_mbuf_.frames;
   vm.frame = mmap_last_capture_buff_;

   //
   // A CORRIGER !!!!
   vm.format = picture_.palette;
   //

   vm.width = v4l2_fmt_.fmt.pix.width;
   vm.height = v4l2_fmt_.fmt.pix.height;
   // v4l
   if (ioctl(device_, VIDIOCMCAPTURE, &vm) < 0) {
      perror("QCamV4L::mmapCapture");
      // AEW: try and do something sensible here - the V4L source
      // has gone away
      close(device_);
      exit(0);
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
