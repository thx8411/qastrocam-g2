/******************************************************************
Qastrocam-g2
Copyright (C) 2010-2014   Blaise-Florentin Collin

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

#ifndef _QCamQHY5_hpp_
#define _QCamQHY5_hpp_

/* only available if have usb */
#if HAVE_USB_H && HAVE_PTHREADS_H

#include <stdio.h>
#include <stdlib.h>

#include <Qt/qslider.h>
#include <Qt/qlabel.h>
#include <Qt/qobject.h>
#include <Qt/qprogressbar.h>

#include "QCamSlider.hpp"
#include "QCam.hpp"
#include "QHY5cam.hpp"

#define QHY5_EXPOSURE_TABLE_SIZE	17

/** QCam implementation to support QHY5 and clones cameras **/
class QCamQHY5 : public QCam {
   Q_OBJECT
public:
   QCamQHY5();
   QCamFrame yuvFrame() const { return yuvBuffer_; }
   const QSize & size() const;
   void resize(const QSize & s);
   ~QCamQHY5();
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
   QHY5cam* camera;
   QCamFrame inputBuffer_;
   QCamFrame yuvBuffer_;
   QTimer* timer_;
   QTimer* progressTimer_;
   QSize targetSize;
   int frameExposure_;
   int xstart_;
   int ystart_;
   int width_;
   int height_;
   int targetWidth_;
   int targetHeight_;
   int gainG1_;
   int gainG2_;
   int gainR_;
   int gainB_;
   int progress_;
   bool shootMode_;
   bool denoise_;
   bool shooting_;
   // gui stuff
   QCamSlider* gainSlider;
   QCamSlider* gainSliderG1;
   QCamSlider* gainSliderB;
   QCamSlider* gainSliderR;
   QCamSlider* gainSliderG2;
   QSlider* exposureSlider;
   QLabel* exposureValue;
   QProgressBar* progressBar;
   QCheckBox* denoiseBox;
   // exposure tab
   static const int exposureTable[QHY5_EXPOSURE_TABLE_SIZE];
protected slots:
   void changeExposure(int e);
   void setExposure();
   void changeGain(int g);
   void changeGainG1(int g);
   void changeGainG2(int g);
   void changeGainR(int g);
   void changeGainB(int g);
   void setGain();
   void progressUpdate();
   void denoiseChange(bool d);
   virtual bool updateFrame();
};

#endif /* HAVE_USB_H && HAVE_PTHREADS_H */

#endif
