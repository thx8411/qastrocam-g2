/******************************************************************
Qastrocam-g2
Copyright (C) 2010   Blaise-Florentin Collin

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

#include <qobject.h>
#include <stdio.h>
#include <stdlib.h>
#include <qlabel.h>
#include "QCam.hpp"

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
public slots:
   virtual bool updateFrame();
signals:
};

#endif
