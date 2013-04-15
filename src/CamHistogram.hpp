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


#ifndef _CamHistogram_hpp_
#define _CamHistogram_hpp_

#include <Qt/qpainter.h>
#include <Qt/qimage.h>
//Added by qt3to4:
#include <Qt/qlabel.h>

#include <stdio.h>
#include <stdlib.h>

#include "QCamClient.hpp"

class QPushButton;
class QWidget;
class Q3VGroupBox;
class Q3ProgressBar;
class QLCDNumber;
class Q3HBox;
class Q3HGroupBox;
class QLabel;
class QHistogram;
class QCam;

#define focusHistorySize_ 200

/** Helper windows to display an histogram. */
class CamHistogram : public QCamClient {
   Q_OBJECT
public:
   CamHistogram(QCam &);
   ~CamHistogram();
   QWidget & widget();
   const QWidget & widget() const;
protected slots:
   void newFrame();
private:
   Q3HGroupBox * mainWindow_;
   Q3VGroupBox * histoGroup_;
   QHistogram * histogramArea_;
   Q3VGroupBox * focusGroup_;
   QPushButton * resetFocus_;
   QWidget * focusLevel_;
   QHistogram * focusArea_;
   Q3HBox * seeingGroup_;
   QLabel * seeingLabel_;
   Q3ProgressBar * seeingLevel_;
   //QLCDNumber * seeingValue_;
   void init();
   //QCam * cam_;
   int histogram_[256];
   int focusIndexValue_;
   double getDistFromNeibourg(int x,int y) const;
};

#endif
