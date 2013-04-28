/******************************************************************
Qastrocam
Copyright (C) 2003-2009   Franck Sicard
Qastrocam-g2
Copyright (C) 2009-2013   Blaise-Florentin Collin

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

#ifndef _QGridBox_h_
#define _QGridBox_h_

#include <Qt/qwidget.h>
#include <Qt/qgridlayout.h>
#include <QtCore/qcoreevent.h>


class QGridBox : public QWidget {
   Q_OBJECT
private:
   // max width or height (depending of the Orientation)
   int size_;
   // current number of inserted elements
   int nbElements_;
   // orientation
   Qt::Orientation orientation_;
   // the grid layout
   QGridLayout layout_;
public:
   QGridBox(QWidget* parent , Qt::Orientation, int size, const char* name = 0 );
   void addWidget(QWidget* w);
   // for children detection
   bool event(QEvent *event);
};

#endif
