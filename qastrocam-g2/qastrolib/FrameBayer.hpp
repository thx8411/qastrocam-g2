/******************************************************************
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


#ifndef _FrameBayer_hpp_
#define _FrameBayer_hpp_

#include <qobject.h>
#include <qstring.h>
#include <qhbox.h>
#include <qlabel.h>

#include "FrameAlgo.hpp"
#include "QCamComboBox.hpp"

class FrameBayer :  public FrameAlgo {
   Q_OBJECT;
private:
    class Widget : public QHBox {
   public:
      ~Widget();
      Widget(QWidget * parent,const FrameBayer * algo);
   private:
      QLabel* label1;
      QLabel* label2;
      QCamComboBox* pattern;
      QCamComboBox* algorithm;
   };
public:
   FrameBayer(QCamTrans* cam);
   bool transform(const QCamFrame in, QCamFrame & out);
   QString label() const {return("Bayer");}
   QWidget * allocGui(QWidget * parent) const {
      return new Widget(parent,this);
   }
public slots:
signals:
};


#endif
