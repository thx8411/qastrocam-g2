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

#include <Qt/qobject.h>
#include <Qt/qstring.h>
#include <Qt/qlabel.h>

#include "QCamHBox.hpp"
#include "FrameAlgo.hpp"
#include "QCamComboBox.hpp"

class FrameBayer;

class BayerWidget : public QCamHBox {
   Q_OBJECT
   public:
      ~BayerWidget();
      BayerWidget(QWidget * parent,const FrameBayer * algo);
   private:
      QWidget* padding1;
      QWidget* padding2;
      QWidget* padding3;
      QLabel* label1;
      QLabel* label2;
      QCamComboBox* pattern;
      QCamComboBox* algorithm;
   private slots :
      void patternChanged(int num);
};


class FrameBayer :  public FrameAlgo {
   Q_OBJECT
   public:
      FrameBayer(QCamTrans* cam);
      bool transform(const QCamFrame in, QCamFrame & out);
      QString label() const {return("Bayer");}
      QWidget * allocGui(QWidget * parent) const {
         return new BayerWidget(parent,this);
      }
   public slots:
      void patternChanged(int num);
      void algorithmChanged(int num);
   private:
      int patternId;
      int methodId;
};


#endif
