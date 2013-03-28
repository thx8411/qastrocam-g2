/******************************************************************
Qastrocam-g2
Copyright (C) 2010   Blaise-Florentin Collin

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

#ifndef _QCamQHY6_hpp_
#define _QCamQHY6_hpp_

#include <qobject.h>
#include <stdio.h>
#include <stdlib.h>
#include <qslider.h>
#include <qlabel.h>
#include <qprogressbar.h>
#include "QCamSlider.hpp"
#include "QCam.hpp"
#include "QHY6cam.hpp"

#define QHY6_EXPOSURE_TABLE_SIZE	17

/** QCam implementation to support QHY5 and clones cameras **/
class QCamQHY6 : public QCam {
   Q_OBJECT
public:
   QCamQHY6();
   QCamFrame yuvFrame() const { return yuvBuffer_; }
   const QSize & size() const;
   void resize(const QSize & s);
   ~QCamQHY6();
   QWidget * buildGUI(QWidget * parent);
protected:
   mutable QSize * sizeTable_;
   virtual const QSize* getAllowedSize() const;
private:
   // functions
   void setSize(int x, int y);

   // slider position to/from exposure in ms
   int getExposureTime(int i);
   int getExposureIndex(int t);

   // vars
   QHY6cam* camera;
   QCamFrame inputBuffer_;
   QCamFrame yuvBuffer_;
   QTimer* timer_;
   QTimer* progressTimer_;
   QSize targetSize;
   //int frameRate_;
   int frameExposure_;
   int width_;
   int height_;
   int targetWidth_;
   int targetHeight_;
   int gain_;
   int progress_;
   bool shootMode_;
   bool shooting_;
   // gui stuff
   QCamSlider* gainSlider;
   QSlider* exposureSlider;
   QLabel* exposureValue;
   QProgressBar* progressBar;
   // exposure tab
   static const int exposureTable[QHY6_EXPOSURE_TABLE_SIZE];
public slots:
protected slots:
   void changeExposure(int e);
   void setExposure();
   void changeGain(int g);
   void setGain();
   void progressUpdate();
   virtual bool updateFrame();
signals:
};

#endif