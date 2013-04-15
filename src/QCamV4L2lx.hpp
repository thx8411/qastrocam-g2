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


#ifndef _QCamV4L2lx_hpp_
#define _QCamV4L2lx_hpp_

#include <Qt/qtimer.h>
#include <Qt/qlabel.h>
#include <Qt/qtooltip.h>

#include "QCamComboBox.hpp"

#include "QCamV4L2.hpp"
/** enhance QCamV4L2 to handle external lx stuff */

// lx modes list
#define lxNone  0
#define lxPar   1
#define lxSer   2

class QCamV4L2lx : public QCamV4L2 {
   Q_OBJECT
public:
   QCamV4L2lx(const char* devpath="/dev/video0");
   QWidget * buildGUI(QWidget * parent);
private:
   // gui
   Q3HGroupBox* remoteCTRLlx;
   // lx mode widgets
   QLabel * lxLabel1;
   QLabel * lxRate;
   QCamComboBox * lxSelector;
   QLabel * lxLabel2;
   QLineEdit * lxTime;
   QPushButton * lxSet;
   Q3ProgressBar * lxBar;
   QWidget* padding;
   QTimer * lxTimer;
   // lx mode vars
   SCmod* lxControler;
   double lxDelay; // integration time
   //double lxFineDelay; // fine tuning for interlace sync
   double lxBaseTime; // os time at integration beginning
   bool lxEnabled; // is lx mode on ?
   int lxFrameCounter; // dropped frames number
   int lxLevel; // level used to decide if a frame should be dropped, based on mean frame luminance
   int lxFramesToDrop;
public slots:
   bool updateFrame();
   // lx mode slots
   void setLXmode(int val);
   void setLXtime();
   void LXframeReady();
};

#endif
