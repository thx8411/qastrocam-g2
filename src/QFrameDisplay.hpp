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


#ifndef _QFrameDisplay_hpp_
#define _QFrameDisplay_hpp_

#include <Qt/qwidget.h>
#include "QCamFrame.hpp"
//Added by qt3to4:
#include <QtGui/QPaintEvent>

class QPainter;

class QPen;

/** Simple widget that can display a QCamFrame */
class QFrameDisplay : public QWidget {
public:
   QFrameDisplay(QWidget * parent,const char * label);
   ~QFrameDisplay();
   void frame(const QCamFrame &);
protected:
   void paintEvent(QPaintEvent * ev);
private:
   QCamFrame frame_;
   QPainter * painter_;
   QPen * pen_;
};

#endif
