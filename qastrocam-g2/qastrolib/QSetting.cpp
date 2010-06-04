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
#include <qmessagebox.h>
#include <qtooltip.h>

#include "QSetting.moc"

#include "../config.h"

#include "QCamUtilities.hpp"
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
   QToolTip::add(videoDeviceEntry,tr("Video device to use, just type 'qhy5' for the QHY5 camera"));
   videoDeviceChooser=new QFileChooser(videoBox,DEVICE_FILE);
   QToolTip::add(videoDeviceChooser,tr("Selects video device"));

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
   int telescopeTable[]={0,1,2,3,4,5,6,7};
   const char* telescopeLabel[]={"none","qhy5","autostar","mcu","mts","apm","fifo","file"};
   telescopeList=new QCamComboBox("telescope type : ",lineOne,8,telescopeTable,telescopeLabel);
   QToolTip::add(telescopeList,tr("Telescope protocol to use for guiding"));
   lineOne->setStretchFactor(telescopeList,10);
   padding1=new QWidget(lineOne);
   lineOne->setStretchFactor(padding1,5);
   telescopeLevels=new QCheckBox("Invert levels",lineOne);
   QToolTip::add(telescopeLevels,tr("Shale we invert TTL levels ?"));
   padding2=new QWidget(lineOne);
   lineOne->setStretchFactor(padding2,5);
   lineTwo=new QHBox(telescopeBox);
   telescopeDeviceLabel=new QLabel("Telescope device/file : ",lineTwo);
   QToolTip::add(telescopeDeviceLabel,tr("Device to use for telescope guiding"));
   telescopeDeviceEntry=new QLineEdit(lineTwo);
   telescopeDeviceChooser=new QFileChooser(lineTwo);

   padding7=new QWidget(remoteCTRL_);
   remoteCTRL_->setStretchFactor(padding7,5);
   // long exposure box
   lxBox=new QHGroupBox("Long exposure",remoteCTRL_);
   remoteCTRL_->setStretchFactor(lxBox,0);
   lxDeviceLabel=new QLabel("Long exposure device : ",lxBox);
   lxDeviceEntry=new QLineEdit(lxBox);
   QToolTip::add(lxDeviceEntry,tr("Device to use for long exposure control"));
   lxDeviceChooser=new QFileChooser(lxBox,DEVICE_FILE);
   QToolTip::add(lxDeviceChooser,tr("Selects long exposure device"));
   lxLevels=new QCheckBox("Invert levels",lxBox);
   QToolTip::add(lxLevels,tr("Shale we invert TTL levels ? (applies to 'toucam led' also)"));

   padding9=new QWidget(remoteCTRL_);
   remoteCTRL_->setStretchFactor(padding9,5);
   // modules box
   modulesBox=new QVGroupBox("Modules",remoteCTRL_);
   QToolTip::add(modulesBox,tr("Modules to activate"));
   remoteCTRL_->setStretchFactor(modulesBox,0);
   lineFive=new QHBox(modulesBox);
   modulesAdd=new QCheckBox("Frame stacking module",lineFive);
   modulesAlign=new QCheckBox("Align frames",lineFive);
   lineSix=new QHBox(modulesBox);
   modulesMax=new QCheckBox("Frame 'ghost' module",lineSix);
   modulesKing=new QCheckBox("King method module",lineSix);

   padding8=new QWidget(remoteCTRL_);
   remoteCTRL_->setStretchFactor(padding8,5);
   // options box
   optionsBox=new QVGroupBox("Options",remoteCTRL_);
   QToolTip::add(optionsBox,tr("Options to activate"));
   remoteCTRL_->setStretchFactor(optionsBox,0);
   lineThree=new QHBox(optionsBox);
#if HAVE_SDL_H
   optionsSdl=new QCheckBox("Use SDL",lineThree);
#endif
   optionsExpert=new QCheckBox("Use expert mode",lineThree);
   lineFour=new QHBox(optionsBox);
   optionsLog=new QCheckBox("Write log file",lineFour);
   optionsNightVision=new QCheckBox("Night vision mode",lineFour);
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
#if HAVE_SDL_H
   connect(optionsSdl,SIGNAL(toggled(bool)),this,SLOT(hasChanged()));
#endif
   connect(optionsExpert,SIGNAL(toggled(bool)),this,SLOT(hasChanged()));
   connect(optionsLog,SIGNAL(toggled(bool)),this,SLOT(hasChanged()));
   connect(modulesAdd,SIGNAL(toggled(bool)),this,SLOT(hasChanged()));
   connect(modulesMax,SIGNAL(toggled(bool)),this,SLOT(hasChanged()));
   connect(modulesKing,SIGNAL(toggled(bool)),this,SLOT(hasChanged()));
   connect(modulesAlign,SIGNAL(toggled(bool)),this,SLOT(hasChanged()));

   connect(optionsNightVision,SIGNAL(toggled(bool)),this,SLOT(setNightVision(bool)));

   // combobox connection
   connect(telescopeList,SIGNAL(activated(int)),this,SLOT(changeTelescope(int)));

   return(remoteCTRL_);
}

const QString & QSetting::label() const {
   return label_;
}

void QSetting::fillFields() {

   // entries
   if(settings.haveKey("VIDEO_DEVICE"))
      videoDeviceEntry->setText(settings.getKey("VIDEO_DEVICE"));
   else {
      videoDeviceEntry->setText("/dev/video0");
      hasChanged();
   }
   if(settings.haveKey("TELESCOPE_DEVICE"))
      telescopeDeviceEntry->setText(settings.getKey("TELESCOPE_DEVICE"));
   else {
     telescopeDeviceEntry->setText("/dev/ttyS1");
      hasChanged();
   }
   if(settings.haveKey("LX_DEVICE"))
      lxDeviceEntry->setText(settings.getKey("LX_DEVICE"));
   else {
      lxDeviceEntry->setText("/dev/ttyS0");
      hasChanged();
   }
   if(settings.haveKey("LIB_PATH"))
      libpathEntry->setText(settings.getKey("LIB_PATH"));
   else {
      libpathEntry->setText(QCamUtilities::basePathName());
      hasChanged();
   }

   // combobox
   if(settings.haveKey("TELESCOPE")) {
      telescopeList->setCurrentText(settings.getKey("TELESCOPE"));
      changeTelescope(-1);
   } else {
      telescopeList->setCurrentText("none");
      changeTelescope(-1);
   }

   // checkboxs
   if(settings.haveKey("TS_LEVELS_INVERTED"))
      telescopeLevels->setChecked(string(settings.getKey("TS_LEVELS_INVERTED"))=="yes");
   else
      hasChanged();
   if(settings.haveKey("LX_LEVELS_INVERTED"))
      lxLevels->setChecked(string(settings.getKey("LX_LEVELS_INVERTED"))=="yes");
   else
      hasChanged();
#if HAVE_SDL_H
   if(settings.haveKey("SDL"))
      optionsSdl->setChecked(string(settings.getKey("SDL"))=="yes");
   else
      hasChanged();
#endif
   if(settings.haveKey("EXPERT"))
      optionsExpert->setChecked(string(settings.getKey("EXPERT"))=="yes");
   else
      hasChanged();
   if(settings.haveKey("LOG"))
      optionsLog->setChecked(string(settings.getKey("LOG"))=="yes");
   else
      hasChanged();
   if(settings.haveKey("ADD_MODULE"))
      modulesAdd->setChecked(string(settings.getKey("ADD_MODULE"))=="yes");
   else
      hasChanged();
   if(settings.haveKey("MAX_MODULE"))
      modulesMax->setChecked(string(settings.getKey("MAX_MODULE"))=="yes");
   else
      hasChanged();
   if(settings.haveKey("KING_MODULE"))
      modulesKing->setChecked(string(settings.getKey("KING_MODULE"))=="yes");
   else
      hasChanged();
   if(settings.haveKey("ALIGN_MODULE"))
      modulesAlign->setChecked(string(settings.getKey("ALIGN_MODULE"))=="yes");
   else
      hasChanged();
   if(settings.haveKey("NIGHT_VISION"))
      optionsNightVision->setChecked(string(settings.getKey("NIGHT_VISION"))=="yes");
}

// slots

void QSetting::saveSettings() {
   QString temp;

   save->setEnabled(false);
   restore->setEnabled(false);

   // save fields

   // entries
   if(!videoDeviceEntry->text().isEmpty())
      settings.setKey("VIDEO_DEVICE",videoDeviceEntry->text().latin1());
   if(!telescopeDeviceEntry->text().isEmpty())
      settings.setKey("TELESCOPE_DEVICE",telescopeDeviceEntry->text().latin1());
   if(!lxDeviceEntry->text().isEmpty())
      settings.setKey("LX_DEVICE",lxDeviceEntry->text().latin1());
   if(!libpathEntry->text().isEmpty())
      settings.setKey("LIB_PATH",libpathEntry->text().latin1());
   // combo
   settings.setKey("TELESCOPE",telescopeList->currentText().latin1());
   // checkboxes
   if(telescopeLevels->isChecked())
      temp="yes";
   else
      temp="no";
   settings.setKey("TS_LEVELS_INVERTED",temp.latin1());
   if(lxLevels->isChecked())
      temp="yes";
   else
      temp="no";
   settings.setKey("LX_LEVELS_INVERTED",temp.latin1());
#if HAVE_SDL_H
   if(optionsSdl->isChecked())
      temp="yes";
   else
      temp="no";
   settings.setKey("SDL",temp.latin1());
#endif
   if(optionsExpert->isChecked())
      temp="yes";
   else
      temp="no";
   settings.setKey("EXPERT",temp.latin1());
   if(optionsLog->isChecked())
      temp="yes";
   else
      temp="no";
   settings.setKey("LOG",temp.latin1());
   if(modulesAdd->isChecked())
      temp="yes";
   else
      temp="no";
   settings.setKey("ADD_MODULE",temp.latin1());
   if(modulesMax->isChecked())
      temp="yes";
   else
      temp="no";
   settings.setKey("MAX_MODULE",temp.latin1());
   if(modulesAlign->isChecked())
      temp="yes";
   else
      temp="no";
   settings.setKey("ALIGN_MODULE",temp.latin1());
   if(modulesKing->isChecked())
      temp="yes";
   else
      temp="no";
   settings.setKey("KING_MODULE",temp.latin1());

   // message box
   QMessageBox::information(0,"Qastrocam-g2","Please restart Qastrocam-g2\nto get the new settings");
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
   // none
   if(telescopeList->currentText()==QString("none")) {
      telescopeDeviceEntry->setEnabled(false);
      telescopeDeviceChooser->setEnabled(false);
      telescopeLevels->setEnabled(false);
   // qhy5 : no device name
   } else if(telescopeList->currentText()==QString("qhy5")) {
      telescopeDeviceEntry->setEnabled(false);
      telescopeDeviceChooser->setEnabled(false);
      telescopeLevels->setEnabled(false);
   // files, no levels
   } else if(telescopeList->currentText()==QString("file")) {
      telescopeDeviceEntry->setEnabled(true);
      telescopeDeviceChooser->setType(REGULAR_FILE);
      telescopeDeviceChooser->setEnabled(true);
      telescopeLevels->setEnabled(false);
   } else if(telescopeList->currentText()==QString("fifo")) {
      telescopeDeviceEntry->setEnabled(false);
      telescopeDeviceEntry->setText("/tmp/qastrocam-g2_shift.fifo");
      telescopeDeviceChooser->setEnabled(false);
      telescopeLevels->setEnabled(false);
   // protoles, devices but no levels
   } else if(telescopeList->currentText()==QString("autostar")||telescopeList->currentText()==QString("mcu")||telescopeList->currentText()==QString("mts")) {
      telescopeDeviceChooser->setType(DEVICE_FILE);
      telescopeDeviceEntry->setEnabled(true);
      telescopeDeviceChooser->setEnabled(true);
      telescopeLevels->setEnabled(false);
   // direct centronic access
   } else {
      telescopeDeviceChooser->setType(DEVICE_FILE);
      telescopeDeviceEntry->setEnabled(true);
      telescopeDeviceChooser->setEnabled(true);
      telescopeLevels->setEnabled(true);
   }
   if(index!=-1)
      hasChanged();
}

void QSetting::setNightVision(bool state) {
   QString temp;

   if(state) {
      QCamUtilities::setNightMode();
      // conf file
   } else {
      QCamUtilities::setStdMode();
      // conf file
   }
    if(optionsNightVision->isChecked())
      temp="yes";
   else
      temp="no";
   settings.setKey("NIGHT_VISION",temp.latin1());
}

// global slot

void QSetting::hasChanged() {
   save->setEnabled(true);
   restore->setEnabled(true);
}
