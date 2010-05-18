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

#ifndef _QCamQHY5_hpp_
#define _QCamQHY5_hpp_

#include <qobject.h>
#include <stdio.h>
#include <stdlib.h>
#include "QCam.hpp"
#include "QHY5cam.hpp"

/** QCam implementation to support QHY5 and clones cameras **/
class QCamQHY5 : public QCam {
   Q_OBJECT
public:
   QCamQHY5();
   QCamFrame yuvFrame() const { return yuvBuffer_; }
   const QSize & size() const;
   void resize(const QSize & s);
   ~QCamQHY5();
   QWidget * buildGUI(QWidget * parent);
protected:
   mutable QSize * sizeTable_;
   virtual const QSize* getAllowedSize() const;
private:
   // functions
   bool setSize(int x, int y);
   // vars
   QHY5cam* camera;
   QCamFrame yuvBuffer_;
public slots:
protected slots:
signals:
};

#endif
