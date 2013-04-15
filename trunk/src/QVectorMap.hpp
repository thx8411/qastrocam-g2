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


#ifndef _QVectorMap_hpp_
#define _QVectorMap_hpp_

#include <Qt/qwidget.h>
#include <Qt3Support/q3valuelist.h>
//Added by qt3to4:
#include <QtGui/QPaintEvent>
#include "Vector2D.hpp"
class QPaintEvent;

enum DrawMode {
   DrawPoint,
   DrawLine
};

class QVectorMap : public QWidget {
   Q_OBJECT
public:
   QVectorMap(QWidget * parent=0, const char * name=0, Qt::WFlags f=0 );
   virtual ~QVectorMap();
 public slots:
   void add(const Vector2D&);
   void reset();
   void setMode(DrawMode);
   void setScale(int scale);
protected:
   void paintEvent( QPaintEvent * ev);
private:
   typedef Q3ValueList<Vector2D> VectorList;
   VectorList vectorList_;
   DrawMode mode_;
   int scale_;
};

#endif
