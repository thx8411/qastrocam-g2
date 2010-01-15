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


#ifndef _QBLINK_HPP_
#define _QBLINK_HPP_

#include <qlabel.h>

// very simple class showing some activity
// eatch time you call the "step" member
// (one char rotating indicator, - \ | /)

class QBlink : public QLabel {
   Q_OBJECT
public :
   QBlink(QWidget* parent);
   void step();
private :
   int position;
   QString items[4];
};

#endif
