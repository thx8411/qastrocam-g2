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


#include "FrameMirror.hpp"
#include <qhbox.h>
#include <qpushbutton.h>
#include <qtooltip.h>

#include "QCamTrans.hpp"

bool FrameMirror::transform(const QCamFrame in, QCamFrame & out) {
   if (in.empty()) {
      return false;
   }
   if (swapUpDown_ || swapLeftRight_ ) {
      out.setMode(in.getMode());
      out.setSize(in.size());
      out.copy(in,
               0,0,
               in.size().width()-1,in.size().height()-1,
               0,0,
               swapLeftRight_,swapUpDown_);
   } else {
      out=in;
   }
   return true;
}

/*void FrameMirror::memswap(unsigned char *dest,
                          const unsigned char *src, size_t n) {
   unsigned i=0,j=n-1;
   while (i<n) {
      dest[i++]=src[j--];
   }
}*/

FrameMirror::FrameMirror(QCamTrans* cam) {
   cam_=cam;
   swapUpDown_=swapLeftRight_=false;
}

void FrameMirror::swapLeftRight(bool val) {
   swapLeftRight_=val;

   emit(leftRightSwapped(val));
}

void FrameMirror::swapUpDown(bool val) {
   swapUpDown_=val;

   emit(upDownSwapped(val));
}

FrameMirror::Widget::Widget(QWidget * parent,const FrameMirror * algo):
   QHBox(parent) {
   upDown_ = new QPushButton(tr("swap Up/Down"),this);
   upDown_->setToggleButton(true);
   connect(upDown_,SIGNAL(toggled(bool)),algo,SLOT(swapUpDown(bool)));
   connect(algo,SIGNAL(upDownSwapped(bool)),upDown_,SLOT(setOn(bool)));

   leftRight_ = new QPushButton(tr("swap Left/Right"),this);
   leftRight_->setToggleButton(true);
   connect(leftRight_,SIGNAL(toggled(bool)),algo,SLOT(swapLeftRight(bool)));
   connect(algo,SIGNAL(leftRightSwapped(bool)),leftRight_,SLOT(setOn(bool)));
   QToolTip::add(upDown_,tr("Swaps frame up/down"));
   QToolTip::add(leftRight_,tr("Swaps frame left/right"));
}

FrameMirror::Widget::~Widget() {
   delete leftRight_;
   leftRight_=NULL;
   delete upDown_;
   upDown_=NULL;
}
