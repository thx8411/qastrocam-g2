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

#include "QCamStack.moc"

#include "../config.h"

QCamStack::QCamStack() {
   label_=QString("Pre-Process");
   camIndex=0;
}

QCamStack::~QCamStack() {
}

void QCamStack::addCam(QCam* cam, QString name) {
   // add the cam if we have room left
   if(camIndex<CAMSTACK_SIZE) {
      nameTab[camIndex]=name;
      camTab[camIndex]=cam;
      camIndex++;
   }
}

QWidget *QCamStack::buildGUI(QWidget * parent) {
   remoteCTRL_= new QVBox(parent);
   // for each cam, create a group
   // and biuld cam gui
   for(int i=0;i<camIndex;i++) {
      groupTab[i]=new QHGroupBox(nameTab[i],remoteCTRL_);
      camTab[i]->buildGUI(groupTab[i]);
   }
   return(remoteCTRL_);
}

const QString & QCamStack::label() const {
   return label_;
}

