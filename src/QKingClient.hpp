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


#ifndef __QKingClient_hpp_
#define __QKingClient_hpp_

#include <time.h>

#include <Qt/qlabel.h>
#include <Qt/qpushbutton.h>
#include <Qt/qtimer.h>

#include "QCamTrans.hpp"
#include "QCamFindShift_hotSpot.hpp"

class QStatusBar;

class QKingClient : public QCamFindShift_hotSpot {
   Q_OBJECT
public:
   QKingClient();
   ~QKingClient();
   QWidget* buildGUI(QWidget *parent);
protected slots:
   void kingStart();
   void kingStop();
   void kingReset();
private:
   // time in second of the firts frame
   time_t firstFrameDate_;
   // positions and shift
   ShiftInfo frameShift_;
   // GUI
   QPushButton* kingStartButton;
   QPushButton* kingStopButton;
   QLabel* statusBar_;
   QCamTrans* kingCam_;
   QTimer* kingTimer_;
private slots:
   void kingRefresh();
};

#endif
