
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


#ifndef _FrameBias_hpp_
#define _FrameBias_hpp_

#include <Qt/qobject.h>
#include <Qt/qstring.h>
#include <Qt/qcheckbox.h>
#include <Qt/qwidget.h>
#include <Qt/qlineedit.h>
#include <Qt/qlabel.h>

#include "QCamHBox.hpp"
#include "FrameAlgo.hpp"
#include "QFileChooser.hpp"

class FrameBias :  public FrameAlgo {
   Q_OBJECT
private:
    class Widget : public QCamHBox {
   public:
      ~Widget();
      Widget(QWidget * parent,const FrameBias * algo);
   private:
      QWidget* padding1;
      QWidget* padding2;
      QWidget* padding3;
      QCheckBox* activate;
      QLabel* label1;
      QFileChooser* fileChooser;
      QLineEdit* fileEntry;
   };
public:
   FrameBias(QCamTrans* cam);
   bool transform(const QCamFrame in, QCamFrame & out);
   QString label() const {return("Dark");}
   QWidget * allocGui(QWidget * parent) const {
      return new Widget(parent,this);
   }
public slots:
   void activatedChange(int s);
   void fileChanged(const QString & name);
signals:
   void desactivated(bool);
private :
   bool activated;
   QString fileName;
   QCamFrame biasFrame;
};


#endif
