/******************************************************************
Qastrocam
Copyright (C) 2003-2009   Franck Sicard
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


#ifndef _QCamAdd_hpp_
#define _QCamAdd_hpp_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <Qt/qobject.h>
#include <Qt/qsize.h>
#include <Qt/qprogressbar.h>
#include <Qt/qgroupbox.h>
#include <Qt/qradiobutton.h>

#include "QCamVGroupBox.hpp"
#include "QCamHGroupBox.hpp"
#include "QCam.hpp"

#define QCAM_ADD_COLOR

// modes
#define QCAM_ADD_ADD		0
#define QCAM_ADD_AVERAGE	1
#define QCAM_ADD_MEDIAN		2

class QCamSlider;
class QImage;
class QCamHBox;
class QPushButton;
class elementaryFrame;
class QCamRadioBox;
class QCamComboBox;
class QCheckBox;

#define MultiSatMode

/** Stacks a serie of frame from an other QCam object.*/
class QCamAdd : public QCam {
   Q_OBJECT
   QCam* cam_;
   /** num total of buffer currently activated (they can
       not be all in use) */
   int numOfActivatedBuffers_;
   /** num total of buffer currently in use */
   int numOfActiveBuffers_;
   bool bufferFull;
   int curBuff_;
   void* integrationBuff_;

   QCamFrame* frameHistory_;
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

   int method_;

   static const int numOfBuffers_;

   mutable bool newIntegrationBuff_;
   double (*funcDisplay_)(double);
   bool negateDisplay_;
   ImageMode mode_;
   void allocBuff(const QSize &);
   void zeroBuff(const QSize & size);
   void addFrame(const QCamFrame &);
   void integration2yuv(const void* integration,QCamFrame & yuv) const ;
   void average2yuv(const void* integration,QCamFrame & yuv) const ;
   void median2yuv(const void* integration,QCamFrame & yuv) const ;

   void removeFrame(const QCamFrame & frame);
   void removeAverageFrame(const QCamFrame & frame);
   void removeMedianFrame(const QCamFrame & frame);

   void addFrame(const QCamFrame & frame,
                 int & maxYValue,
                 int & minYValue,
                 int & maxCrValue);
   void averageFrame(const QCamFrame & frame);
   void medianFrame(const QCamFrame & frame);

   void moveFrame(const QCamFrame & frame,
                  int & maxYValue,
                  int & minYValue,
                  int & maxCrValue,
                  const bool adding);
   /* for remote control */
   QGroupBox* methodWidget_;
   QRadioButton* frameSum;
   QRadioButton* frameAverage;
   QRadioButton* frameMedian;

   QCamHGroupBox* accumulationWidget_;
   QCamComboBox* remoteCTRLnumOfActiveBuffer_;
   QProgressBar* bufferFill_;
   QPushButton* resetBufferFill_;
   QCamVGroupBox* displayOptions_;
   QCamSlider* remoteCTRLmaxYvalue_;
   QCamSlider* remoteCTRLminYvalue_;
   QCamRadioBox* modeDisplayButton_;
   QCheckBox* invDisplayButton_;
#ifdef MultiSatMode
   QCamRadioBox* maxCrSaturatedButton_;
#else
   QCheckBox* maxCrSaturatedButton_;
#endif
public:
   QCamAdd(QCam* cam);
   const QSize & size() const { return cam_->size();}
   void resize(const QSize & s) {cam_->resize(s);}
   virtual const QSize * getAllowedSize() const { return QCam::getAllowedSize();}
   ~QCamAdd();
   QWidget* buildGUI(QWidget * parent);
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
   void methodChanged(bool b);
};

#endif

