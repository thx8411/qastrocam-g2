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


#include "QTelescope.moc"

#include <qlayout.h>
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qpushbutton.h>

#include "QCamUtilities.hpp"

QTelescope::QTelescope() {
   mainWidget_=NULL;
   arrowsLayout_=NULL;
   arrows_=NULL;
   upButton_=NULL;
   downButton_=NULL;
   leftButton_=NULL;
   rightButton_=NULL;
}

void QTelescope::buildGUI(QWidget * parent) {
   QPixmap* tmpIcon;
   mainWidget_=new QVGroupBox("Telescope CTRL",parent);

   QCamUtilities::registerWidget(mainWidget_);

   QCamUtilities::setQastrocamIcon(mainWidget_);
   arrows_ = new QWidget(mainWidget_);
   arrowsLayout_=new QGridLayout(arrows_,3,3);
   upButton_=new QPushButton(arrows_,"U");
   tmpIcon=QCamUtilities::getIcon("up.png");
   upButton_->setPixmap(*tmpIcon);
   delete tmpIcon;
   downButton_=new QPushButton(arrows_,"D");
   tmpIcon=QCamUtilities::getIcon("down.png");
   downButton_->setPixmap(*tmpIcon);
   delete tmpIcon;
   leftButton_=new QPushButton(arrows_,"L");
   tmpIcon=QCamUtilities::getIcon("left.png");
   leftButton_->setPixmap(*tmpIcon);
   delete tmpIcon;
   rightButton_=new QPushButton(arrows_,"R");
   tmpIcon=QCamUtilities::getIcon("right.png");
   rightButton_->setPixmap(*tmpIcon);
   delete tmpIcon;
   arrowsLayout_->addWidget(upButton_,0,1);
   arrowsLayout_->addWidget(downButton_,2,1);
   arrowsLayout_->addWidget(leftButton_,1,0);
   arrowsLayout_->addWidget(rightButton_,1,2);
   upButton_->show();
   arrows_->show();
   connect(upButton_,SIGNAL(pressed()),this,SLOT(goN()));
   connect(upButton_,SIGNAL(released()),this,SLOT(stopN()));
   connect(downButton_,SIGNAL(pressed()),this,SLOT(goS()));
   connect(downButton_,SIGNAL(released()),this,SLOT(stopS()));
   connect(leftButton_,SIGNAL(pressed()),this,SLOT(goE()));
   connect(leftButton_,SIGNAL(released()),this,SLOT(stopE()));
   connect(rightButton_,SIGNAL(pressed()),this,SLOT(goW()));
   connect(rightButton_,SIGNAL(released()),this,SLOT(stopW()));
   mainWidget_->show();
}

QTelescope::~QTelescope() {
   QCamUtilities::removeWidget(mainWidget_);
}

QWidget * QTelescope::widget() {
   return mainWidget_;
}
