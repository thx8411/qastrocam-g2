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


#ifndef _QCamAutoAlign_hpp_
#define _QCamAutoAlign_hpp_

#include <stdio.h>

#include <string>

#include "QCam.hpp"
#include "ShiftInfo.hpp"

class QPushButton;
class QCamFindShift;
class ShiftInfo;
class QHistogram;
class QVectorMap;
class QCamSlider;
class Q3HGroupBox;
class QCheckBox;

/** realign the frames of an other QCam object.
    It is usefull if the tracking is bad (or not existant).
*/
class QCamAutoAlign : public QCam {
   Q_OBJECT
public:
   QCamAutoAlign();
   QCamFrame yuvFrame() const;
   void setTracker(QCamFindShift * tracker);
   virtual const QSize & size() const;
   virtual void resize(const QSize &s);
   virtual const QSize * getAllowedSize() const { return QCam::getAllowedSize();}
public slots:
   void reset();
protected slots:
   void shifted(const ShiftInfo & );
 private slots:
   void setCropValue(int crop) {cropValue_=crop/100.0;}
   void setImageCenter(bool center) {center_=center;}
private:
   QWidget * buildGUI(QWidget *);
   /** connect cam and tracker.
    */
   void shiftFrame(const ShiftInfo & shift,
                   const QCamFrame orig,
                   QCamFrame & shifted,
                   float crop,bool center);
   bool connectEveryThing();
   void unConnectEveryThing();
   QCamFindShift * tracker_;
   ShiftInfo currentShift_;
   QCamFrame yuvFrame_;
   Q3HGroupBox * findShiftWidget_;
   QWidget * findShiftCtrl_;
#define ONE_MAP 1
#if ONE_MAP
   QVectorMap * shiftMap_;
#else
   QHistogram * shiftXhisto_;
   QHistogram * shiftYhisto_;
#endif
   QCamSlider * scaleSlider_;
   QCheckBox * centerButton_;
   QCamSlider * cropSlider_;
   bool center_;
   float cropValue_;
   QSize currentSize_;
   string fifoName_;
   FILE * fifo_;
};
#endif
