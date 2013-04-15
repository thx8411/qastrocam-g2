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


#ifndef _QCamVesta_hpp_
#define _QCamVesta_hpp_

// only available for kernel 3
#if !KERNEL_2

#include <Qt/qobject.h>
#include <Qt/qimage.h>

#include <stdio.h>
#include <stdlib.h>
#include <linux/videodev2.h>

#include "QCamV4L2.hpp"

class QCamSlider;
class Q3HGroupBox;
class QCamRadioBox;
class QLCDNumber;
class QCheckBox;
class Q3ProgressBar;
class QLineEdit;
class SCmod;
class SCmodTucLed;

/** enhance QCamV4L2 to access the specific features of the Philips
    Vesta and TOUCAM webcams.
    give acces to the mod 1 and mod 2 of the Steve Chambers hardware patch. */
class QCamVesta : public QCamV4L2 {
   Q_OBJECT
public:
   QCamVesta(const char * devpath="/dev/video0");
   ~QCamVesta();
   int getGain() const;
   int getExposure() const;
   //int getCompression() const;
   bool getFlicker() const;
   bool getBackLight() const;
   int getNoiseRemoval() const;
   int getSharpness() const;
   int getGama() const;
   int getFrameRate() const;
   int getType() const {return type_;}
   void setSCmodImpl(SCmod *impl) {SCmodCtrl_=impl;}
protected:
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
   Q3HGroupBox * remoteCTRLframeRate_;
   QCamComboBox * remoteCTRLframeRate2_;
   enum SCmodType { SCmodNone,SCmodPPort,SCmodLed,SCmodSerial, SCmodPPort2};
   QCamComboBox * SCmodSelector_;
   QLineEdit * longExposureTime_;
   Q3ProgressBar * exposureTimeLeft_;
   QLCDNumber * exposureTime_;
   int multiplicateur_;
   int skippedFrame_;
   int type_;
   SCmod * SCmodCtrl_;

public slots:
   void setGain(int value);
   void setExposure(int value);
   //void setCompression(int value);
   void setNoiseRemoval(int value);
   void setSharpness(int value);
   /* shortcut to :
       - setSharpness(0);
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
protected slots:
   bool updateFrame();

signals:
   void gainChange(int);
   //void compressionChange(int);
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

#endif /* !KERNEL_2 */

#endif
