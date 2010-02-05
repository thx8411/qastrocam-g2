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
   videoDeviceLabel=new QLabel("Video device : ",videoBox);
   videoDeviceEntry=new QLineEdit(videoBox);
   videoDeviceChooser=new QFileChooser(videoBox);

   // telescope box
   telescopeBox=new QVGroupBox("Telescope control",remoteCTRL_);
   remoteCTRL_->setStretchFactor(telescopeBox,10);
   lineOne=new QHBox(telescopeBox);
   telescopeListLabel=new QLabel("Protocol : ",lineOne);
   int telescopeTable[]={0,1,2,3,4,5};
   const char* telescopeLabel[]={"apm","autostar","fifo","mcu","mts","file"};
   telescopeList=new QCamComboBox("telescope type : ",lineOne,6,telescopeTable,telescopeLabel);
   telescopeLevels=new QCheckBox("Invert levels",lineOne);
   lineTwo=new QHBox(telescopeBox);
   telescopeDeviceLabel=new QLabel("Telescope device/file : ",lineTwo);
   telescopeDeviceEntry=new QLineEdit(lineTwo);
   telescopeDeviceChooser=new QFileChooser(lineTwo);

   // long exposure box
   lxBox=new QHGroupBox("Longue exposure",remoteCTRL_);
   remoteCTRL_->setStretchFactor(lxBox,10);
   lxDeviceLabel=new QLabel("Long exposure device : ",lxBox);
   lxDeviceEntry=new QLineEdit(lxBox);
   lxDeviceChooser=new QFileChooser(lxBox);
   lxLevels=new QCheckBox("Invert levels",lxBox);

   // options box
   optionsBox=new QVGroupBox("Options",remoteCTRL_);
   remoteCTRL_->setStretchFactor(optionsBox,10);
   lineThree=new QHBox(optionsBox);
   //optionsGrid=new QGridLayout(optionsBox,2,2);
   optionsSdl=new QCheckBox("Use SDL",lineThree);
   //optionsGrid->addWidget(optionsSdl,0,0);
   optionsExpert=new QCheckBox("Use expert mode",lineThree);
   //optionsGrid->addWidget(optionsSdl,0,1);
   lineFour=new QHBox(optionsBox);
   optionsLog=new QCheckBox("Write log file",lineFour);
   //optionsGrid->addWidget(optionsSdl,1,0);
   optionsForceGeneric=new QCheckBox("Force use of generic cam",lineFour);
   //optionsGrid->addWidget(optionsSdl,1,1);*/
   libBox=new QHBox(optionsBox);
   libpathLabel=new QLabel("Library path : ",libBox);
   libpathEntry=new QLineEdit(libBox);
   libpathChooser=new QDirectoryChooser(libBox);

   // buttons
   buttonsBox=new QHBox(remoteCTRL_);
   remoteCTRL_->setStretchFactor(buttonsBox,0);
   save=new QPushButton("Save",buttonsBox);
   restore->setEnabled(false);
   restore= new QPushButton("Restore",buttonsBox);
   restore->setEnabled(false);

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
