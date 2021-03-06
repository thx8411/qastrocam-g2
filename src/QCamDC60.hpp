/******************************************************************
Qastrocam-g2
Copyright (C) 2009-2013 Blaise-Florentin Collin

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


#ifndef _QCamDC60_hpp_
#define _QCamDC60_hpp_

#include <Qt/qcheckbox.h>
#include <Qt/qlabel.h>
#include <Qt/qtimer.h>
#include <Qt/qprogressbar.h>

#include "QCamV4L2.hpp"

/* enhance QCamV4L2 to handle specifities of the AstroEasyCap device driver. */

class QCamDC60 : public QCamV4L2 {
   Q_OBJECT
public:
   QCamDC60(const char * devpath="/dev/video0");
   void setIntegration(int val);
   void setInverted(bool b);
protected :
   QWidget *  buildGUI(QWidget * parent);
private:
   // vars
   int progress;
   float lxDelay;
   QTimer* progressTimer;
   // gui
   // lx
   QCheckBox* lxCheck;
   QLabel* lxLabel;
   QLineEdit* lxEntry;
   QPushButton* lxSet;
   QProgressBar* lxProgress;
   // extras
   QCheckBox* extraPreamp;
   QCheckBox* extraAntialias;
   QCheckBox* extraWhitepeak;
private slots :
   // extras
   void preampChanged(int b);
   void antialiasChanged(int b);
   void whitepeakChanged(int b);
   // lx
   void lxActivated(int b);
   void lxSetPushed();
   void lxProgressStep();
};

#endif
