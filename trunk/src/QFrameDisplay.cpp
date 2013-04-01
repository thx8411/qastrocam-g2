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


#include "QFrameDisplay.hpp"
#include "QCamFrame.hpp"
#include <qcolor.h>
#include <qpen.h>
#include <qpainter.h>

QFrameDisplay::QFrameDisplay(QWidget * parent,const char * label) :
   QWidget(parent,label) {
   painter_ = new QPainter();
   pen_=new QPen();
   pen_->setStyle(DotLine);
   pen_->setColor(QColor(255,0,0));
   setWFlags(WRepaintNoErase);
}

QFrameDisplay::~QFrameDisplay() {
   delete pen_;
   delete painter_;
}

void QFrameDisplay::frame(const QCamFrame &frame) {
   frame_=frame;
   setMinimumSize(frame_.size());
   resize(frame_.size());
   update();
}

void QFrameDisplay::paintEvent(QPaintEvent * ev) {
   if (!frame_.empty()) {
      painter_->begin(this);
      painter_->setPen(*pen_);
      painter_->setClipRegion(ev->region());
      painter_->drawImage(0,0,frame_.colorImage());
      painter_->drawLine(width()/2,0,
                         width()/2,height());
      painter_->drawLine(0,height()/2,
                         width(),height()/2);
      painter_->end();
   }
}
