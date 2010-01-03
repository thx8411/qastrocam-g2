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


#ifndef _QCamAdd_hpp_
#define _QCamAdd_hpp_

#include <qobject.h>

#include "QCam.hpp"
#include <qsize.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define QCAM_ADD_COLOR

class QCamSlider;
class QImage;
class QHBox;
class QPushButton;
class elementaryFrame;
class QProgressBar;
class QCamRadioBox;
class QCamComboBox;
class QCheckBox;
class QVGroupBox;
class QHGroupBox;

#define MultiSatMode

/** Stacks a serie of frame from an other QCam object.*/
class QCamAdd : public QCam {
   Q_OBJECT
   QCam * cam_;
   /** num total of buffer currently activated (they can
       not be all in use) */
   int numOfActivatedBuffers_;
   /** num total of buffer currently in use */
   int numOfActiveBuffers_;
   int curBuff_;
   int * integrationBuff_;

   QCamFrame * frameHistory_;
   mutable QCamFrame computedFrame_;
   QSize curSize_;
   int maxYValue_;
   int minYValue_;
   int maxCrValue_;
#ifdef MultiSatMode
   int maxCrValueAutoSaturated_;
#else
   bool maxCrValueAutoSaturated_;
#endif
   bool maxYValueAuto_;
   bool minYValueAuto_;

   static const int numOfBuffers_;

   mutable bool newIntegrationBuff_;
   double (*funcDisplay_)(double);
   bool negateDisplay_;
   ImageMode mode_;
   void allocBuff(const QSize &);
   void zeroBuff(const QSize & size);
   void addFrame(const QCamFrame &);
   bool writeFit(const QString & name,int * buff) const;
   bool loadFit(const QString & name,int * buff) const;
   void integration2yuv(const int * integration,
                        QCamFrame & yuv ) const ;
   void removeFrame(const QCamFrame & frame);
   void addFrame(const QCamFrame & frame,
                 int & maxYValue,
                 int & minYValue,
                 int & maxCrValue);
   void moveFrame(const QCamFrame & frame,
                  int & maxYValue,
                  int & minYValue,
                  int & maxCrValue,
                  const bool adding);
   /* for remote control */
   QHGroupBox * accumulationWidget_;
   QCamComboBox *remoteCTRLnumOfActiveBuffer_;
   QProgressBar * bufferFill_;
   QPushButton * resetBufferFill_;
   QVGroupBox * displayOptions_;
   QCamSlider *remoteCTRLmaxYvalue_;
   QCamSlider *remoteCTRLminYvalue_;
   QCamRadioBox * modeDisplayButton_;
   QCheckBox * invDisplayButton_;
#ifdef MultiSatMode
   QCamRadioBox * maxCrSaturatedButton_;
#else
   QCheckBox * maxCrSaturatedButton_;
#endif
public:
   QCamAdd(QCam* cam);
   const QSize & size() const { return cam_->size();}
   void resize(const QSize & s) {cam_->resize(s);}
   virtual const QSize * getAllowedSize() const { return QCam::getAllowedSize();}
   ~QCamAdd();
   QWidget * buildGUI(QWidget * parent);
   QCamFrame yuvFrame() const;
public slots:
   void setNumOfBuffer(int nbuf);
   void setMaxYvalue(int);
   void setMinYvalue(int);
   void resetBufferFill();
   void modeDisplay(int val);
   void negateDisplay(bool val) {negateDisplay_=val;}
#ifdef  MultiSatMode
   void maxSaturatedColors(int val) {maxCrValueAutoSaturated_=val;}
#else
   void maxSaturatedColors(bool val) {maxCrValueAutoSaturated_=val;}
#endif
 signals:
   void numOfBufferChange(int);
   void maxYValueChange(int);
   void minYValueChange(int);
 private slots:
   void addNewFrame();
};

#endif

