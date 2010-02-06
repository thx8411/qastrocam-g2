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

#include "SettingsBackup.hpp"

// settings object, needed everywhere
extern settingsBackup settings;

QSetting::QSetting() {
   label_=QString("Settings");
}

QSetting::~QSetting() {
}

QWidget *QSetting::buildGUI(QWidget * parent) {
   remoteCTRL_= new QVBox(parent);
   remoteCTRL_->setSpacing(4);

   padding3=new QWidget(remoteCTRL_);
   remoteCTRL_->setStretchFactor(padding3,5);
   // video device box
   videoBox=new QHGroupBox("Video device",remoteCTRL_);
   remoteCTRL_->setStretchFactor(videoBox,0);
   videoDeviceLabel=new QLabel("Video device : ",videoBox);
   videoDeviceEntry=new QLineEdit(videoBox);
   videoDeviceChooser=new QFileChooser(videoBox,DEVICE_FILE);

   padding6=new QWidget(remoteCTRL_);
   remoteCTRL_->setStretchFactor(padding6,5);
   // telescope box
   telescopeBox=new QVGroupBox("Telescope control",remoteCTRL_);
   remoteCTRL_->setStretchFactor(telescopeBox,0);
   lineOne=new QHBox(telescopeBox);
   padding0=new QWidget(lineOne);
   lineOne->setStretchFactor(padding0,5);
   telescopeListLabel=new QLabel("Protocol : ",lineOne);
   lineOne->setStretchFactor(telescopeListLabel,0);
   int telescopeTable[]={0,1,2,3,4,5};
   const char* telescopeLabel[]={"apm","autostar","fifo","mcu","mts","file"};
   telescopeList=new QCamComboBox("telescope type : ",lineOne,6,telescopeTable,telescopeLabel);
   lineOne->setStretchFactor(telescopeList,10);
   padding1=new QWidget(lineOne);
   lineOne->setStretchFactor(padding1,5);
   telescopeLevels=new QCheckBox("Invert levels",lineOne);
   padding2=new QWidget(lineOne);
   lineOne->setStretchFactor(padding2,5);
   lineTwo=new QHBox(telescopeBox);
   telescopeDeviceLabel=new QLabel("Telescope device/file : ",lineTwo);
   telescopeDeviceEntry=new QLineEdit(lineTwo);
   telescopeDeviceChooser=new QFileChooser(lineTwo);

   padding7=new QWidget(remoteCTRL_);
   remoteCTRL_->setStretchFactor(padding7,5);
   // long exposure box
   lxBox=new QHGroupBox("Longue exposure",remoteCTRL_);
   remoteCTRL_->setStretchFactor(lxBox,0);
   lxDeviceLabel=new QLabel("Long exposure device : ",lxBox);
   lxDeviceEntry=new QLineEdit(lxBox);
   lxDeviceChooser=new QFileChooser(lxBox,DEVICE_FILE);
   lxLevels=new QCheckBox("Invert levels",lxBox);

   padding8=new QWidget(remoteCTRL_);
   remoteCTRL_->setStretchFactor(padding8,5);
   // options box
   optionsBox=new QVGroupBox("Options",remoteCTRL_);
   remoteCTRL_->setStretchFactor(optionsBox,0);
   lineThree=new QHBox(optionsBox);
   optionsSdl=new QCheckBox("Use SDL",lineThree);
   optionsExpert=new QCheckBox("Use expert mode",lineThree);
   lineFour=new QHBox(optionsBox);
   optionsLog=new QCheckBox("Write log file",lineFour);
   optionsForceGeneric=new QCheckBox("Force use of generic cam",lineFour);
   libBox=new QHBox(optionsBox);
   libpathLabel=new QLabel("Library path : ",libBox);
   libpathEntry=new QLineEdit(libBox);
   libpathChooser=new QDirectoryChooser(libBox);

   padding4=new QWidget(remoteCTRL_);
   remoteCTRL_->setStretchFactor(padding4,5);
   // buttons
   buttonsBox=new QHBox(remoteCTRL_);
   remoteCTRL_->setStretchFactor(buttonsBox,5);
   save=new QPushButton("Save",buttonsBox);
   save->setEnabled(false);
   restore= new QPushButton("Restore",buttonsBox);
   restore->setEnabled(false);

   // padding
   padding5=new QWidget(remoteCTRL_);
   remoteCTRL_->setStretchFactor(padding5,5);

   // fill all the fields
   fillFields();

   // buttons connections
   connect(save,SIGNAL(released()),this,SLOT(saveSettings()));
   connect(restore,SIGNAL(released()),this,SLOT(restoreSettings()));

   // choosers connections
   connect(videoDeviceChooser,SIGNAL(fileChanged(const QString &)),this,SLOT(changeVideoDevice(const QString &)));
   connect(telescopeDeviceChooser,SIGNAL(fileChanged(const QString &)),this,SLOT(changeTelescopeDevice(const QString &)));
   connect(lxDeviceChooser,SIGNAL(fileChanged(const QString &)),this,SLOT(changeLxDevice(const QString &)));
   connect(libpathChooser,SIGNAL(directoryChanged(const QString &)),this,SLOT(changeLibpath(const QString &)));

   // entries connection
   connect(videoDeviceEntry,SIGNAL(textChanged(const QString &)),this,SLOT(hasChanged()));
   connect(telescopeDeviceEntry,SIGNAL(textChanged(const QString &)),this,SLOT(hasChanged()));
   connect(lxDeviceEntry,SIGNAL(textChanged(const QString &)),this,SLOT(hasChanged()));
   connect(libpathEntry,SIGNAL(textChanged(const QString &)),this,SLOT(hasChanged()));

   // checkboxs connection
   connect(telescopeLevels,SIGNAL(toggled(bool)),this,SLOT(hasChanged()));
   connect(lxLevels,SIGNAL(toggled(bool)),this,SLOT(hasChanged()));
   connect(optionsSdl,SIGNAL(toggled(bool)),this,SLOT(hasChanged()));
   connect(optionsExpert,SIGNAL(toggled(bool)),this,SLOT(hasChanged()));
   connect(optionsLog,SIGNAL(toggled(bool)),this,SLOT(hasChanged()));
   connect(optionsForceGeneric,SIGNAL(toggled(bool)),this,SLOT(hasChanged()));

   // combobox connection
   connect(telescopeList,SIGNAL(activated(int)),this,SLOT(hasChanged()));

   return(remoteCTRL_);
}

const QString & QSetting::label() const {
   return label_;
}

void QSetting::fillFields() {
   // TODO
}

// slots

void QSetting::saveSettings() {
   save->setEnabled(false);
   restore->setEnabled(false);

   // save fields
   // TODO
}

void QSetting::restoreSettings() {
   save->setEnabled(false);
   restore->setEnabled(false);

   // restore fields
   fillFields();
}

// entries slots

void QSetting::changeVideoDevice(const QString& name) {
   videoDeviceEntry->setText(name);
}

void QSetting::changeTelescopeDevice(const QString& name) {
   telescopeDeviceEntry->setText(name);
}

void QSetting::changeLxDevice(const QString& name) {
   lxDeviceEntry->setText(name);
}

void QSetting::changeLibpath(const QString& name) {
   libpathEntry->setText(name);
}

// combox slot

void QSetting::changeTelescope(int index) {
   hasChanged();
}

// global slot

void QSetting::hasChanged() {
   save->setEnabled(true);
   restore->setEnabled(true);
}
