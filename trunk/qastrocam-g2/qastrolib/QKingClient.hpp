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


#ifndef __QKingClient_hpp_
#define __QKingClient_hpp_

#include "QCamFindShift_hotSpot.hpp"
#include <time.h>

class QStatusBar;

class QKingClient : public QCamFindShift_hotSpot {
   Q_OBJECT;
public:
   QKingClient();
   QWidget * buildGUI(QWidget *parent);
protected:
   bool registerFirstFrame();
   bool findShift(ShiftInfo & shift);
private:
   /** time in second of the firts frame */
   time_t timeFirstFrame_;
   QStatusBar * statusBar_;
};

#endif
