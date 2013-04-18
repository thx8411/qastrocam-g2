/******************************************************************
Qastrocam-g2
Copyright (C) 2010-2013   Blaise-Florentin Collin

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

#ifndef _QCamSimulator_hpp_
#define _QCamSimulator_hpp_

#include <stdio.h>
#include <stdlib.h>

#include <Qt/qobject.h>
#include <Qt/qlabel.h>
#include <Qt/qpushbutton.h>

#include "QCamSlider.hpp"
#include "QCam.hpp"

#define _SIMULATOR_STOP_	0
#define _SIMULATOR_UP_		1
#define _SIMULATOR_DOWN_	2
#define _SIMULATOR_LEFT_	3
#define _SIMULATOR_RIGHT_	4

/** QCam Simulator **/
class QCamSimulator : public QCam {
   Q_OBJECT
public:
   QCamSimulator();
   QCamFrame yuvFrame() const { return yuvBuffer_; }
   const QSize & size() const;
   void resize(const QSize & s);
   ~QCamSimulator();
   QWidget * buildGUI(QWidget * parent);
protected:
   mutable QSize * sizeTable_;
   virtual const QSize* getAllowedSize() const;
private:
   QCamFrame yuvBuffer_;
   QTimer* timer_;
   double starPositionX_;
   double starPositionY_;
   double raSpeed_;
   double decSpeed_;
   int raMove_;
   int decMove_;
   // GUI
   QPushButton* raLeft_;
   QPushButton* raRight_;
   QPushButton* raStop_;
   QPushButton* raCenter_;
   QPushButton* decUp_;
   QPushButton* decDown_;
   QPushButton* decStop_;
   QPushButton* decCenter_;
   QCamSlider* raSpeedSlider_;
   QCamSlider* decSpeedSlider_;
private slots :
   void moveLeft(bool s);
   void moveRight(bool s);
   void stopRa();
   void moveUp(bool s);
   void moveDown(bool s);
   void stopDec();
   void setRaSpeed(int s);
   void setDecSpeed(int s);
   void centerRa();
   void centerDec();
public slots:
   virtual bool updateFrame();
signals:
};

#endif
