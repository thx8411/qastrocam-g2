
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


#ifndef _FrameDark_hpp_
#define _FrameDark_hpp_

#include <qobject.h>
#include <qstring.h>
#include <qhbox.h>
#include <qcheckbox.h>
#include <qwidget.h>
#include <qlineedit.h>
#include <qlabel.h>

#include "FrameAlgo.hpp"
#include "QFileChooser.hpp"

class FrameDark :  public FrameAlgo {
   Q_OBJECT;
private:
    class Widget : public QHBox {
   public:
      ~Widget();
      Widget(QWidget * parent,const FrameDark * algo);
   private:
      QWidget* padding1;
      QWidget* padding2;
      QWidget* padding3;
      QWidget* padding4;
      QCheckBox* activate;
      QLabel* label1;
      QLabel* label2;
      QFileChooser* fileChooser;
      QLineEdit* fileEntry;
      QLineEdit* timeEntry;
   };
public:
   FrameDark(QCamTrans* cam);
   bool transform(const QCamFrame in, QCamFrame & out);
   QString label() const {return("Dark");}
   QWidget * allocGui(QWidget * parent) const {
      return new Widget(parent,this);
   }
public slots:
   void activatedChange(int s);
   void fileChanged(const QString & name);
   void timeChanged(const QString & timeF);
signals:
   void desactivated(bool);
private :
   bool activated;
   QString fileName;
   QString timeString;
   QCamFrame darkFrame;
   double timeFactor;
};


#endif
