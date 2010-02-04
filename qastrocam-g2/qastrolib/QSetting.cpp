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

#include <qvbox.h>

#include "QSetting.moc"

QSetting::QSetting() {
   label_=QString("Settings");
}

QSetting::~QSetting() {
}

QWidget *QSetting::buildGUI(QWidget * parent) {
   remoteCTRL_= new QVBox(parent);
   remoteCTRL_->setSpacing(4);

   // video device box
   videoBox=new QHGroupBox("Video device",remoteCTRL_);
   remoteCTRL_->setStretchFactor(videoBox,10);

   // telescope box
   telescopeBox=new QVGroupBox("Telescope control",remoteCTRL_);
   remoteCTRL_->setStretchFactor(telescopeBox,10);

   // long exposure box
   lxBox=new QVGroupBox("Longue exposure",remoteCTRL_);
   remoteCTRL_->setStretchFactor(lxBox,10);

   // options box
   optionsBox=new QVGroupBox("Options",remoteCTRL_);
   remoteCTRL_->setStretchFactor(optionsBox,10);

   // buttons
   buttonsBox=new QHBox(remoteCTRL_);
   remoteCTRL_->setStretchFactor(buttonsBox,0);
   save=new QPushButton("Save",buttonsBox);
   restore= new QPushButton("Restore",buttonsBox);

   connect(save,SIGNAL(released()),this,SLOT(saveSettings()));
   connect(restore,SIGNAL(released()),this,SLOT(restoreSettings()));

   return(remoteCTRL_);
}

const QString & QSetting::label() const {
   return label_;
}

void QSetting::saveSettings() {
}

void QSetting::restoreSettings() {
}
