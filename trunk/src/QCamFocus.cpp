/******************************************************************
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

#include <Qt/qlayout.h>

#include "QCamFocus.hpp"


QCamFocus::QCamFocus(QCam* cam) {
   label_=QString("Focus");
   cam_=cam;
   sel_=NULL;
   connect(cam_,SIGNAL(newFrame()),this,SLOT(focusNewFrame()));

//
   //sel_=new QCamSelection();
   //sel_->connectCam(*cam_);
   //sel_->widget().show();
   //connect(sel_->impl(),SIGNAL(selectionChanged()),this,SLOT(newSelection()));
//
}

QCamFocus::~QCamFocus() {
   delete sel_;
}

QWidget *QCamFocus::buildGUI(QWidget * parent) {
//
   //FocusPlot* plot;
//

   remoteCTRL_= new Q3VBox(parent);


//
   //plot=new FocusPlot();

   //remoteCTRL_->layout()->add(plot);

   //plot->show();
//

   return(remoteCTRL_);
}

const QString & QCamFocus::label() const {
   return label_;
}

void QCamFocus::focusNewFrame() {
}

void QCamFocus::newSelection() {
   //
   //cout << sel_->getSelectionCenterX() << ":" << sel_->getSelectionCenterY() << endl;
   //
}

//
/*
FocusPlot::FocusPlot()
{
  setTitle("Focus zone");

  setRotation(30,0,15);
  setScale(1,1,1);
  setShift(0.15,0,0);
  setZoom(0.9);

  for (unsigned i=0; i!=coordinates()->axes.size(); ++i)
  {
    coordinates()->axes[i].setMajors(7);
    coordinates()->axes[i].setMinors(4);
  }


  coordinates()->axes[X1].setLabelString("x-axis");
  coordinates()->axes[Y1].setLabelString("y-axis");
  coordinates()->axes[Z1].setLabelString(QChar (0x38f)); // Omega - see http://www.unicode.org/charts/

  setCoordinateStyle(BOX);

  updateData();
  updateGL();
}
*/
//
