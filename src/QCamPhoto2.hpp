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

#ifndef _QCamPhoto2_hpp_
#define _QCamPhoto2_hpp_

#include <Qt/qobject.h>
#include <stdio.h>
#include <stdlib.h>
#include "QCam.hpp"

/** QCam implementation to access libgphoto2 cameras **/
class QCamPhoto2 : public QCam {
   Q_OBJECT
public:
   QCamPhoto2();
   QCamFrame yuvFrame() const { return yuvBuffer_; }
   const QSize & size() const;
   void resize(const QSize & s);
   ~QCamPhoto2();
   QWidget * buildGUI(QWidget * parent);
protected:
   mutable QSize * sizeTable_;
   virtual const QSize* getAllowedSize() const;
private:
   bool setSize(int x, int y);
   QCamFrame yuvBuffer_;
   uchar * tmpBuffer_;
   QTimer * timer_;
   QSize size_;
public slots:
protected slots:
signals:
};

#endif
