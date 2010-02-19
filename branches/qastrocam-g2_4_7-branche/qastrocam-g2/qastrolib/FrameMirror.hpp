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


#ifndef _FrameMirror_hpp_
#define _FrameMirror_hpp_

#include <qobject.h>
#include <qstring.h>
#include <qhbox.h>

#include "FrameAlgo.hpp"
class QPushButton;

class FrameMirror :  public FrameAlgo {
   Q_OBJECT;
private:
   class Widget : public QHBox {
   public:
      ~Widget();
      Widget(QWidget * parent,const FrameMirror * algo);
   private:
      QPushButton * upDown_;
      QPushButton * leftRight_;
   };

public:
   FrameMirror();
   bool transform(const QCamFrame in, QCamFrame & out);
   QString label() const {return "Mirror";}
   QWidget * allocGui(QWidget * parent) const {
      return new Widget(parent,this);
   }
public slots:
   void swapUpDown(bool);
   void swapLeftRight(bool);
signals:
   void upDownSwapped(bool);
   void leftRightSwapped(bool);
private:
   void memswap(unsigned char *dest,
                const unsigned char *src, size_t n);
   bool swapUpDown_;
   bool swapLeftRight_;
};

#endif
