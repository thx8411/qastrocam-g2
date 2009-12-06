#ifndef _QCamV4L_hpp_
#define _QCamV4L_hpp_

#include <qobject.h>
#include <qimage.h>
#include <qhgroupbox.h>
#include <qlineedit.h>
#include <qprogressbar.h>
#include <qpushbutton.h>

#include <stdio.h>
#include <stdlib.h>
#include <linux/videodev.h>

#include "QCam.hpp"
#include "QBlink.hpp"

#include "SCmod.hpp"

class QCamSlider;
class QTimer;
class QSocketNotifier;

// lx modes list
#define	lxNone	0
#define lxPar	1
#define lxSer	2

// video ctrl struct
struct video_ctrl {
   int brightness;
   int brightness_min;
   int brightness_max;
   int hue;
   int hue_min;
   int hue_max;
   int contrast;
   int contrast_min;
   int contrast_max;
   int colour;
   int colour_min;
   int colour_max;
   int whiteness;
   int whiteness_min;
   int whiteness_max;
};

// mmap buffer
struct mmap_buffer {
   void *start;
   size_t length;
};

/** QCam implementation to acces a basic Video4Linux device.*/
class QCamV4L : public QCam {
   Q_OBJECT
public:
   // image controls
   enum Options {
      ioNoBlock=(1<<0),
      ioUseSelect=(1<<1),
      haveBrightness=(1<<2),
      haveContrast=(1<<3),
      haveHue=(1<<4),
      haveColor=(1<<5),
      haveWhiteness=(1<<6),
      sendJpegFrames=(1<<7)
   };
   static const int DefaultOptions;
   // create the best camera instance, depending on the device
   static QCam * openBestDevice(const char * devpath = "/dev/video0",const char * devsource = "",bool force = false);
   // constructor
   QCamV4L(const char * devpath="/dev/video0",
           int preferedPalette = 0 /* auto palette*/,
           const char* devsource=NULL,
           unsigned long options =  DefaultOptions /* cf QCamV4L::options */);
   // destructor
   ~QCamV4L();
   // image access
   QCamFrame yuvFrame() const { return yuvBuffer_; }
   int yuvFrameMemSize;
   const QSize & size() const;
   void resize(const QSize & s);
   int getBrightness() const;
   int getContrast() const;
   int getColor() const;
   int getHue() const;
   int getWhiteness() const;
   // gui
   QWidget * buildGUI(QWidget * parent);
protected:
   // V4L2 vars
   struct v4l2_capability v4l2_cap_;
   struct v4l2_format v4l2_fmt_;
   // V4L vars
   int device_;
   unsigned long options_;
   //struct video_window window_;
   struct video_ctrl picture_;
   // mmap stuf
   struct v4l2_requestbuffers mmap_reqbuf;
   struct mmap_buffer* buffers;
   bool useMmap;
   // sizes table
   mutable QSize * sizeTable_;
   // pictures settings
   void updatePictureSettings();
   virtual void refreshPictureSettings();
   // drop a frame
   bool dropFrame();
   /** should be overloaded.
       return the avaibles capture size.
   */
   virtual const QSize* getAllowedSize() const;
   /** return the frame rate of the camera.
       is used if the select() system call is not avaible,
       by the timer used to probe the camera for a new frame */
   virtual int getFrameRate() const { return(frameRate_);}

   // gui
   QHGroupBox * remoteCTRLlx;
   QCamSlider * lxSlider;

private:
   // get os time in seconds (usec accuracy)
   double getTime();
   // V4L2 vars
   v4l2_input input;
   // palette name
   QString palette;
   // device settings stored
   int deviceSource;
   // device name
   string devpath_;
   // basic operations
   bool setSize(int x, int y);
   void init(int preferedPalette);
   void allocBuffers();
   // mmap functions
   bool mmapInit();
   uchar* mmapCapture();
   void mmapRelease();
   // gui
   QCamComboBox*  frameModeB;
   // image frame and buffer
   QCamFrame yuvBuffer_;
   uchar * tmpBuffer_;
   // to probe the cam for new frame
   //QTimer * timer_;
   // to probe the cam if it support select
   QSocketNotifier * notifier_;
   QSize size_;
   /* for remote controle */
   QCamSlider * remoteCTRLbrightness_;
   QCamSlider * remoteCTRLcontrast_;
   QCamSlider * remoteCTRLhue_;
   QCamSlider * remoteCTRLcolor_;
   QCamSlider * remoteCTRLwhiteness_;

   // palette and input display
   QHGroupBox * infoBox;
   QLabel * infoLabel1;
   QLabel * infoPalette;
   QLabel * infoLabel2;
   QLabel * infoInput;

   // lx mode widgets
   //QCamSlider * lxSlider;   -> cf. protected
   //QHGroupBox * remoteCTRLlx;   -> cf. protected
   QLabel * lxLabel1;
   QLabel * lxRate;
   QCamComboBox * lxSelector;
   QLabel * lxLabel2;
   QLineEdit * lxTime;
   QPushButton * lxSet;
   QProgressBar * lxBar;
   QBlink* lxBlink;
   QTimer * lxTimer;
   // video stream vars
   ImageMode mode_;
   int frameRate_;
   // lx mode vars
   SCmod* lxControler;
   double lxDelay; // integration time
   //double lxFineDelay; // fine tuning for interlace sync
   double lxBaseTime; // os time at integration beginning
   bool lxEnabled; // is lx mode on ?
   int lxFrameCounter; // dropped frames number
   int lxLevel; // level used to decide if a frame should be dropped, based on mean frame luminance

public slots:
   // controls
   void setContrast(int value);
   void setBrightness(int value);
   void setColor(int value);
   void setHue(int value);
   void setWhiteness(int value);
   void setMode(ImageMode val);
   void setMode(int val); /* proxy to ' void setMode(ImageMode val)' */
   // lx mode slots
   void setLXmode(int val);
   void setLXtime();
   void LXframeReady();
   void LXlevel(int level);
protected slots:
   // a new frame is up
   virtual bool updateFrame();
signals:
   // controls
   void contrastChange(int);
   void brightnessChange(int);
   void colorChange(int value);
   void hueChange(int value);
   void whitenessChange(int value);
};

#endif
