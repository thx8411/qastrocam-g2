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


#ifndef _QCamSilder_h_
#define _QCamSilder_h_

#include <qobject.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qslider.h>

class QCamSlider : public QHBox {
   Q_OBJECT
public:
   QCamSlider(const QString & label,
              bool needCheckBox =false , QWidget * parent = 0 ,
              int minVal=0, int maxVal=65535,bool noSliderMove=false,
              bool displayPercent=true);
   void setMinValue(int min);
   void setMaxValue(int max);
private:
   QString labelTxt_;
   QLabel * label_;
   QLabel * valueLabel_;
   QCheckBox * checkBox_;
   QSlider * slider_;
   bool noSliderMove_;
   bool percent_;
   int lastEmit_;
private slots:
   void sliderRelease();
   void sliderMove(int val);
   void sliderMoveKey(int val);
   void buttonToggled(bool state);
   void polish();
 public slots:
   void setValue(int val);
 signals:
   void sliderReleased();
   void valueChange(int);
};

#endif
