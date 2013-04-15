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


#ifndef _QCamAutoGuidageSimple_hpp_
#define _QCamAutoGuidageSimple_hpp_

#include "QCamAutoGuidage.hpp"
#include "math.h"

class ShiftInfo;

#include <Qt/qlineedit.h>
#include <Qt/qlcdnumber.h>
#include <Qt/qlabel.h>
#include <Qt3Support/q3hbox.h>

/** basic tracking class.
    Do no prediction, dosn't do any calibration.
    Just try to follow the target.
    Need no mirror plan on the path from the lens to the
    cam. And the cam must not be upsidedown.
*/
class QCamAutoGuidageSimple : public QCamAutoGuidage {
   Q_OBJECT
public:
   QCamAutoGuidageSimple();
   virtual QWidget * buildGUI(QWidget *parent=0);
protected slots:
   /** Swap east and west */
   void swapEW(bool swap);
   /** Swap south and north */
   void swapNS(bool swap);
   /** center the target */
   void setCenter(bool center);
   void frameShift(const ShiftInfo & shift);
   void setAltMinShift(double v) { minAltShift_=v;}
   void setAltMaxShift(double v) { maxAltShift_=v;}
   void setAscMinShift(double v) { minAscShift_=v;}
   void setAscMaxShift(double v) { maxAscShift_=v;}
   void setMinShift(int v) { setMinShift((double)v);}
   void setMaxShift(int v) { setMaxShift((double)v);}
signals:
   void shiftAlt(double);
   void shiftAsc(double);
private:
   bool nsSwapped_;
   bool ewSwapped_;
   bool centerMode_;
   double minAltShift_;
   double maxAltShift_;
   double minAscShift_;
   double maxAscShift_;
};

//
//
//

class TrackingControl : public Q3HBox {
   Q_OBJECT
   int min_;
   int max_;
public:
   TrackingControl(QString label, QWidget * parent=0, const char * name=0, Qt::WFlags f=0 );
   void init();
public slots:
   void setShift(double shift);
   void setMin(int min);
   void setMax(int max);
   void setMin(const QString &);
   void setMax(const QString &);
   void setMoveDir(MoveDir);
signals:
   void minChanged(double);
   void maxChanged(double);
private:
   QLabel label_;
   QLabel arrow_;
   QLCDNumber currentShift_;
   QLabel labelMin_;
   QLineEdit minShift_;
   QLabel labelMax_;
   QLineEdit maxShift_;
   QString keyMin_;
   QString keyMax_;
};

#endif
