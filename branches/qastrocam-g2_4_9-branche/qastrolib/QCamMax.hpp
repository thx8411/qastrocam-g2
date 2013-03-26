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


#ifndef _QCamMax_hpp_
#define _QCamMax_hpp_

#include <qobject.h>


#include "QCam.hpp"
#include <qsize.h>

class QImage;

/** Stacks a serie of frame from .*/
class QCamMax : public QCam {
   Q_OBJECT
   QCam * cam_;
   void allocBuff(const QSize &);
   void addFrame(const uchar *);
   QCamFrame yuvFrame_;
   bool paused_;
public:
   QCamMax(QCam * cam);
   QCamFrame yuvFrame() const { return yuvFrame_; }
   const QSize & size() const { return cam_->size();}
   void resize(const QSize & s) {cam_->resize(s);}
   virtual const QSize * getAllowedSize() const { return QCam::getAllowedSize();}
   ~QCamMax() {};
   QWidget * buildGUI(QWidget * parent);
 public slots:
   void clear();
   void pause();
 private slots:
   void addNewFrame();
};

#endif
