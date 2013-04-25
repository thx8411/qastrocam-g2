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

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>

#include <iostream>

#include <Qt/qwidget.h>
#include <Qt/qpushbutton.h>
#include <Qt/qlabel.h>
#include <Qt/qtooltip.h>
#include <Qt/qprogressbar.h>

#include <Qt3Support/q3hbox.h>

#include "CamHistogram.hpp"
#include "QCam.hpp"
#include "QHistogram.hpp"
#include "QCamUtilities.hpp"

CamHistogram::CamHistogram(QCam & theCam) :
   QCamClient(theCam) {
   mainWindow_ = new QCamHGroupBox(tr("Analyse"));

   QCamUtilities::registerWidget(mainWindow_);

   QCamUtilities::setQastrocamIcon(mainWindow_);

   for (int i=0;i<256;++i) {
      histogram_[i]=0;
   }

   QSizePolicy sizePolicyMax;
   sizePolicyMax.setVerData(QSizePolicy::Expanding);
   sizePolicyMax.setHorData(QSizePolicy::Expanding);

   QSizePolicy sizePolicyMin;
   sizePolicyMin.setVerData(QSizePolicy::Minimum);
   sizePolicyMin.setHorData(QSizePolicy::Minimum);
   mainWindow_->setSizePolicy(sizePolicyMin);

   focusGroup_ = new QCamVGroupBox(tr("Focus - Seeing"), mainWindow_);
   focusArea_ = new QHistogram(focusGroup_);
   focusArea_->setDataSize(focusHistorySize_);
   focusArea_->setAutoShift(true);
   focusArea_->setAverage(3);
   QToolTip::add(focusArea_,tr("Help to focus. High value implies:\nHigh contrast (sharper image)"));

   focusArea_->setMinimumSize(focusHistorySize_,80);
   resetFocus_ = new QPushButton(tr("reset"),focusGroup_);
   focusGroup_->setSizePolicy(sizePolicyMin);
   focusArea_->setSizePolicy(sizePolicyMax);

   seeingGroup_= new Q3HBox(focusGroup_);
   seeingLabel_= new QLabel(tr("Seeing: "),seeingGroup_);
   seeingLevel_=  new QProgressBar(seeingGroup_);
   seeingLevel_->setMinimum(0);
   seeingLevel_->setMaximum(100);
   QToolTip::add(seeingLevel_,tr("Shown an evaluation of the seeing by looking how\nthe contrast changes between frames."));

   connect(resetFocus_,SIGNAL(pressed()),focusArea_,SLOT(reset()));
   histoGroup_ = new QCamVGroupBox(tr("Histogram"), mainWindow_);
   histogramArea_ = new QHistogram(histoGroup_);
   histogramArea_->setDataSize(256);
   histogramArea_->setAverage(5);
   histogramArea_->displayMode(QHistogram::LogDisplay);
   histogramArea_->setMinimumSize(256,120);
   histoGroup_->setSizePolicy(sizePolicyMin);
   histogramArea_->setSizePolicy(sizePolicyMax);
   QToolTip::add(histogramArea_,
                 tr("show an histogram of the image.\n"
                    "0 on the left, 255 on the right"));
   focusGroup_->show();
   histoGroup_->show();
   histogramArea_->show();
   focusArea_->show();
   mainWindow_->show();
}

CamHistogram::~CamHistogram() {
   QCamUtilities::removeWidget(mainWindow_);
}

QWidget & CamHistogram::widget() {
   return * mainWindow_;
}

const QWidget & CamHistogram::widget() const  {
   return * mainWindow_;
}

void CamHistogram::newFrame() {
   // 1) collect histogram
   double distSum=0;
   int w=cam().size().width();
   int h=cam().size().height();
   int wh=w*h;
   const uchar * tab=cam().yuvFrame().Y();

   for (int i=0;i<256;++i) {
      histogram_[i]=0;
   }

   for(int i=(wh-1);
       i>=0;--i) {
      ++histogram_[tab[i]];
   }
   for (int i=0;i<256;++i) {
     histogramArea_->setValue(histogram_[i],i);
   }
   histogramArea_->update();

   int max=0;
   for(int i=(wh-1-w-1);
       i>=0;--i) {
      if ( (i%w) == (w-1)) continue;
      distSum+=abs(tab[i+1]-tab[i])+abs(tab[i+w]-tab[i]);
      if (tab[i]>max) max=tab[i];
   }

   focusArea_->setValue(distSum*distSum*distSum);
   focusArea_->update();

   double seeing=0;
   double maxForSeeing=0;
   for(int i=5; i<10; ++i) {
      if (focusArea_->value(i) > maxForSeeing) {
         maxForSeeing=focusArea_->value(i);
      }
      seeing+=fabs(focusArea_->value(i-5,i+5)-focusArea_->value(i));
   }
   seeing/=maxForSeeing;
   seeingLevel_->setValue(100-((((int)(1000*seeing)-1))/10));
}

double CamHistogram::getDistFromNeibourg(int x,int y) const {
   int ref = cam().getY(x,y);
   return (abs(ref-cam().getY(x+1,y))
           + abs(ref-cam().getY(x,y+1)))/(double)ref;
}
