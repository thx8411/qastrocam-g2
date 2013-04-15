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


#ifndef _FrameId_hpp_
#define _FrameId_hpp_

#include <Qt/qobject.h>
#include <Qt/qstring.h>
#include <Qt3Support/q3hbox.h>

#include "FrameAlgo.hpp"

class FrameId :  public FrameAlgo {
   Q_OBJECT
private:
    class Widget : public Q3HBox {
   public:
      ~Widget();
      Widget(QWidget * parent,const FrameId * algo);
   private:
   };
public:
   FrameId(QCamTrans* cam);
   bool transform(const QCamFrame in, QCamFrame & out);
   QString label() const {return("Output");}
   QWidget * allocGui(QWidget * parent) const {
      return new Widget(parent,this);
   }
public slots:
signals:
};

#endif
