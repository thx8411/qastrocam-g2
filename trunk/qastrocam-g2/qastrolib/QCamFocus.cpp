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

#include "QCamFocus.moc"

#include "../config.h"

QCamFocus::QCamFocus(QCam* cam) {
   label_=QString("Focus");
   cam_=cam;
   connect(cam_,SIGNAL(newFrame()),this,SLOT(focusNewFrame()));
}

QCamFocus::~QCamFocus() {
}

QWidget *QCamFocus::buildGUI(QWidget * parent) {
   remoteCTRL_= new QVBox(parent);
   return(remoteCTRL_);
}

const QString & QCamFocus::label() const {
   return label_;
}

void QCamFocus::focusNewFrame() {
}
