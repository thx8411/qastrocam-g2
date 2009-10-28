#ifndef _QCamVesta_hpp_
#define _QCamVesta_hpp_

#include <qobject.h>
#include <qimage.h>

#include <stdio.h>
#include <stdlib.h>
#include <linux/videodev.h>

#include "pwc-ioctl.h"

#include "QCamV4L.hpp"

class QCamSlider;
class QHGroupBox;
class QCamRadioBox;
class QLCDNumber;
class QCheckBox;
class QProgressBar;
class QLineEdit;
class SCmod;
class SCmodTucLed;

/** enhance QCamV4L to access the specific features of the Philips
    Vesta and TOUCAM webcams.
    give acces to the mod 1 and mod 2 of the Steve Chambers hardware patch. */
class QCamVesta : public QCamV4L {
   Q_OBJECT
public:
   QCamVesta(const char * devpath="/dev/video0");
   ~QCamVesta();
   int getGain() const;
   int getExposure() const;
   int getCompression() const;
   bool getFlicker() const;
   bool getBackLight() const;
   int getNoiseRemoval() const;
   int getSharpness() const;
   int getGama() const;
   int getFrameRate() const;
   int getType() const {return type_;}
   void setSCmodImpl(SCmod *impl) {SCmodCtrl_=impl;}
   const QSize * getAllowedSize() const;
protected:
   struct video_window window_;
   QWidget *  buildGUI(QWidget * parent);
   virtual void refreshPictureSettings();
   friend class SCmodTucLed;
   friend class SCmoduSC2Led;

private:
   void init();
   void initRemoteControlLongExposure(QWidget * remoteCTRL);
   void getWhiteBalance();
   void setWhiteBalance();
   void setLed(int on, int off) const;
   void stopAccumulation();
   void startAccumulation();
   mutable bool haveLeds_;
   bool liveWhiteBalance_;
   bool refreshGui_;
   //struct pwc_whitebalance whitebalance_;
   int whiteBalanceMode_;
   int whiteBalanceRed_;
   int whiteBalanceBlue_;
   mutable int lastGain_;
   /* for remote controle */
   QCamSlider * remoteCTRLgama_;
   QCamSlider * remoteCTRLgain_;
   QCamRadioBox * remoteCTRLWhiteBalance_;
   QCamSlider * remoteCTRLWBred_;
   QCamSlider * remoteCTRLWBblue_;
   QCamSlider * remoteCTRLexposure_;
   QCamSlider * remoteCTRLcompression_;
   QCamSlider * remoteCTRLnoiseRemoval_;
   QCamSlider * remoteCTRLsharpness_;
   QHGroupBox * remoteCTRLframeRate_;
   QCamComboBox * remoteCTRLframeRate2_;
   enum SCmodType { SCmodNone,SCmodPPort,SCmodLed,SCmodSerial, SCmodPPort2};
   QCamComboBox * SCmodSelector_;
   QLineEdit * longExposureTime_;
   QProgressBar * exposureTimeLeft_;
   QLCDNumber * exposureTime_;
   int multiplicateur_;
   int skippedFrame_;
   int type_;
   SCmod * SCmodCtrl_;
   //bool rawBayerMode_;
public slots:
   void setGain(int value);
   void setExposure(int value);
   void setCompression(int value);
   void setNoiseRemoval(int value);
   void setSharpness(int value);
   /** shortcut to :
       - setSharpness(0);
       - setCompression(0);
       - setNoiseRemoval(0);
   */
   void setBestQuality();
   void setGama(int value);
   void setFrameRate(int value);
   void setSCmod(int value);
   void setLongExposureTime(const QString& str);
   void setFrameRateMultiplicateur(int value);
   void setWhiteBalanceMode(int val);
   void setWhiteBalanceRed(int val);
   void setWhiteBalanceBlue(int val);
   void saveSettings();
   void restoreSettings();
   void restoreFactorySettings();
   void setLiveWhiteBalance(bool val);
   void setBackLight(bool val);
   void setFlicker(bool val);
   //void activateRawBayerMode(bool val);
protected slots:
   bool updateFrame();

signals:
   void gainChange(int);
   void exposureChange(int);
   void compressionChange(int);
   void noiseRemovalChange(int);
   void sharpnessChange(int);
   void gamaChange(int);
   void frameRateChange(int);
   void frameRateMultiplicateurChange(int);
   void exposureTime(double);
   void whiteBalanceModeChange(int);
   void whiteBalanceRedChange(int);
   void whiteBalanceBlueChange(int);
};

#endif
