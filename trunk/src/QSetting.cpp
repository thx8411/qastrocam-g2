/******************************************************************
Qastrocam-g2
Copyright (C) 2009-2014   Blaise-Florentin Collin

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

#include <Qt/qlabel.h>
#include <Qt/qmessagebox.h>
#include <Qt/qtooltip.h>

#include "QCamVGroupBox.hpp"
#include "QGridBox.hpp"
#include "QSetting.hpp"
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
   int cameraNumber;
   int telescopeNumber;
   remoteCTRL_= new QCamVBox(parent);

   // video device box
   videoBox=new QCamHGroupBox("Camera",remoteCTRL_);
   camLabel=new QLabel("Camera : ",videoBox);
#if HAVE_USB_H && HAVE_PTHREADS_H
   cameraNumber=4;
   int cameraTable[]={0,1,2,3};
   const char* cameraLabel[]={"simulator","qhy5","qhy6","v4l(2)"};
#else
   cameraNumber=2;
   int cameraTable[]={0,1};
   const char* cameraLabel[]={"simulator","v4l(2)"};
#endif
   cameraList=new QCamComboBox("camera : ",videoBox,cameraNumber,cameraTable,cameraLabel);
   cameraList->setToolTip(tr("Camera to use"));
   videoDeviceLabel=new QLabel("Device : ",videoBox);
   videoDeviceEntry=new QLineEdit(videoBox);
   videoDeviceEntry->setToolTip(tr("Video device to use"));
   videoDeviceChooser=new QFileChooser(videoBox,DEVICE_FILE);
   videoDeviceChooser->setToolTip(tr("Selects video device"));

   // telescope box
   telescopeBox=new QCamVGroupBox("Mount control",remoteCTRL_);
   lineOne=new QCamHBox(telescopeBox);
   telescopeListLabel=new QLabel("Protocol : ",lineOne);
#if HAVE_USB_H && HAVE_PTHREADS_H
   telescopeNumber=12;
   int telescopeTable[]={0,1,2,3,4,5,6,7,8,9,10,11};
   const char* telescopeLabel[]={"none","qhy5","qhy6","autostar","lx200","nexstar","mcu","mts","apm","fifo","file","simulator"};
#else
   telescopeNumber=10;
   int telescopeTable[]={0,1,2,3,4,5,6,7,8,9};
   const char* telescopeLabel[]={"none","autostar","lx200","nexstar","mcu","mts","apm","fifo","file","simulator"};
#endif
   telescopeList=new QCamComboBox("Mount type : ",lineOne,telescopeNumber,telescopeTable,telescopeLabel);
   telescopeList->setToolTip(tr("Mount protocol to use for guiding"));
   QWidget* padding=new QWidget(lineOne);
   telescopeLevels=new QCheckBox("Invert levels",lineOne);
   telescopeLevels->setToolTip(tr("Shale we invert TTL levels ?"));

   lineTwo=new QCamHBox(telescopeBox);
   telescopeDeviceLabel=new QLabel("Mount device/file : ",lineTwo);
   telescopeDeviceLabel->setToolTip(tr("Device to use for mount guiding"));
   telescopeDeviceEntry=new QLineEdit(lineTwo);
   telescopeDeviceChooser=new QFileChooser(lineTwo);

   // long exposure box
   lxBox=new QCamHGroupBox("Long exposure",remoteCTRL_);
   lxDeviceLabel=new QLabel("Long exposure device : ",lxBox);
   lxDeviceEntry=new QLineEdit(lxBox);
   //lxDeviceEntry->setMinimumWidth(72);
   lxDeviceEntry->setToolTip(tr("Device to use for long exposure control"));
   lxDeviceChooser=new QFileChooser(lxBox,DEVICE_FILE);
   lxDeviceChooser->setToolTip(tr("Selects long exposure device"));
   lxLevels=new QCheckBox("Invert levels",lxBox);
   lxLevels->setToolTip(tr("Shale we invert TTL levels ? (applies to 'toucam led' also)"));

   // modules box
   modulesBox=new QCamVGroupBox("Modules",remoteCTRL_);
   modulesBox->setToolTip(tr("Modules to activate"));
   QGridBox* modulesGrid=new QGridBox(modulesBox,Qt::Vertical,2);
   modulesAdd=new QCheckBox("Frame stacking module",modulesGrid);
   modulesAlign=new QCheckBox("Align frames",modulesGrid);
   modulesMax=new QCheckBox("Frame 'ghost' module",modulesGrid);
   modulesKing=new QCheckBox("King method module",modulesGrid);

   // options box
   optionsBox=new QCamVGroupBox("Options",remoteCTRL_);
   optionsBox->setToolTip(tr("Options to activate"));
   QGridBox* optionsGrid=new QGridBox(optionsBox,Qt::Vertical,2);
#if HAVE_SDL_H
   optionsSdl=new QCheckBox("Use SDL",optionsGrid);
#endif
#if KERNEL_2
   optionsExpert=new QCheckBox("Use Vesta expert mode",optionsGrid);
#endif
   optionsLog=new QCheckBox("Write log file",optionsGrid);
   optionsNightVision=new QCheckBox("Night vision mode",optionsGrid);
   optionsRegistax=new QCheckBox("Registax AVI compatibility",optionsGrid);
   libBox=new QCamHBox(optionsBox);
   libpathLabel=new QLabel("Library path : ",libBox);
   libpathEntry=new QLineEdit(libBox);
   libpathChooser=new QDirectoryChooser(libBox);

   // buttons
   buttonsBox=new QCamHBox(remoteCTRL_);
   save=new QPushButton("Save",buttonsBox);
   save->setEnabled(false);
   restore= new QPushButton("Restore",buttonsBox);
   restore->setEnabled(false);

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
#if KERNEL_2
   connect(optionsExpert,SIGNAL(toggled(bool)),this,SLOT(hasChanged()));
#endif
   connect(optionsLog,SIGNAL(toggled(bool)),this,SLOT(hasChanged()));
   connect(optionsRegistax,SIGNAL(toggled(bool)),this,SLOT(hasChanged()));
   connect(modulesAdd,SIGNAL(toggled(bool)),this,SLOT(hasChanged()));
   connect(modulesMax,SIGNAL(toggled(bool)),this,SLOT(hasChanged()));
   connect(modulesKing,SIGNAL(toggled(bool)),this,SLOT(hasChanged()));
   connect(modulesAlign,SIGNAL(toggled(bool)),this,SLOT(hasChanged()));

   connect(optionsNightVision,SIGNAL(toggled(bool)),this,SLOT(setNightVision(bool)));

   // combobox connection
   connect(telescopeList,SIGNAL(activated(int)),this,SLOT(changeTelescope(int)));
   connect(cameraList,SIGNAL(activated(int)),this,SLOT(changeCamera(int)));

   return(remoteCTRL_);
}

const QString & QSetting::label() const {
   return label_;
}

void QSetting::fillFields() {

   // entries
   if(settings.haveKey("CAMERA_DEVICE"))
      videoDeviceEntry->setText(settings.getKey("CAMERA_DEVICE"));
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
      QString* path;
      path=new QString(QCamUtilities::basePathName().c_str());
      libpathEntry->setText(*path);
      hasChanged();
   }

   // combobox
   if(settings.haveKey("TELESCOPE")) {
      telescopeList->setCurrentIndex(telescopeList->getPosition(settings.getKey("TELESCOPE")));
      changeTelescope(-1);
   } else {
      telescopeList->setCurrentIndex(0);
      changeTelescope(-1);
   }

   if(settings.haveKey("CAMERA")) {
      cameraList->setCurrentIndex(cameraList->getPosition(settings.getKey("CAMERA")));
      changeCamera(-1);
   } else {
      cameraList->setCurrentIndex(0);
      changeCamera(-1);
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
#if KERNEL_2
   if(settings.haveKey("EXPERT"))
      optionsExpert->setChecked(string(settings.getKey("EXPERT"))=="yes");
   else
      hasChanged();
#endif
   if(settings.haveKey("LOG"))
      optionsLog->setChecked(string(settings.getKey("LOG"))=="yes");
   else
      hasChanged();
   if(settings.haveKey("REGISTAX_AVI"))
      optionsRegistax->setChecked(string(settings.getKey("REGISTAX_AVI"))=="yes");
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
      settings.setKey("CAMERA_DEVICE",videoDeviceEntry->text().toLatin1());
   if(!telescopeDeviceEntry->text().isEmpty())
      settings.setKey("TELESCOPE_DEVICE",telescopeDeviceEntry->text().toLatin1());
   if(!lxDeviceEntry->text().isEmpty())
      settings.setKey("LX_DEVICE",lxDeviceEntry->text().toLatin1());
   if(!libpathEntry->text().isEmpty())
      settings.setKey("LIB_PATH",libpathEntry->text().toLatin1());
   // combo
   settings.setKey("TELESCOPE",telescopeList->currentText().toLatin1());
   settings.setKey("CAMERA",cameraList->currentText().toLatin1());
   // checkboxes
   if(telescopeLevels->isChecked())
      temp="yes";
   else
      temp="no";
   settings.setKey("TS_LEVELS_INVERTED",temp.toLatin1());
   if(lxLevels->isChecked())
      temp="yes";
   else
      temp="no";
   settings.setKey("LX_LEVELS_INVERTED",temp.toLatin1());
#if HAVE_SDL_H
   if(optionsSdl->isChecked())
      temp="yes";
   else
      temp="no";
   settings.setKey("SDL",temp.toLatin1());
#endif
#if KERNEL_2
   if(optionsExpert->isChecked())
      temp="yes";
   else
      temp="no";
   settings.setKey("EXPERT",temp.toLatin1());
#endif
   if(optionsLog->isChecked())
      temp="yes";
   else
      temp="no";
   settings.setKey("LOG",temp.toLatin1());
   if(optionsRegistax->isChecked())
      temp="yes";
   else
      temp="no";
   settings.setKey("REGISTAX_AVI",temp.toLatin1());
   if(modulesAdd->isChecked())
      temp="yes";
   else
      temp="no";
   settings.setKey("ADD_MODULE",temp.toLatin1());
   if(modulesMax->isChecked())
      temp="yes";
   else
      temp="no";
   settings.setKey("MAX_MODULE",temp.toLatin1());
   if(modulesAlign->isChecked())
      temp="yes";
   else
      temp="no";
   settings.setKey("ALIGN_MODULE",temp.toLatin1());
   if(modulesKing->isChecked())
      temp="yes";
   else
      temp="no";
   settings.setKey("KING_MODULE",temp.toLatin1());

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
   // qhy5 or qhy6 : no device name
   } else if((telescopeList->currentText()==QString("qhy5"))||(telescopeList->currentText()==QString("qhy6"))) {
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
   } else if(telescopeList->currentText()==QString("lx200")||telescopeList->currentText()==QString("autostar")||telescopeList->currentText()==QString("nexstar")||telescopeList->currentText()==QString("mcu")||telescopeList->currentText()==QString("mts")) {
      telescopeDeviceChooser->setType(DEVICE_FILE);
      telescopeDeviceEntry->setEnabled(true);
      telescopeDeviceChooser->setEnabled(true);
      telescopeLevels->setEnabled(false);
    // simulator
    } else if(telescopeList->currentText()==QString("simulator")) {
      telescopeDeviceEntry->setEnabled(false);
      telescopeDeviceChooser->setEnabled(false);
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

void QSetting::changeCamera(int index) {
   if((cameraList->currentText()==QString("simulator"))||(cameraList->currentText()==QString("qhy5"))||(cameraList->currentText()==QString("qhy6"))) {
      videoDeviceEntry->setEnabled(false);
      videoDeviceChooser->setEnabled(false);
   } else {
      videoDeviceEntry->setEnabled(true);
      videoDeviceChooser->setEnabled(true);
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
   settings.setKey("NIGHT_VISION",temp.toLatin1());
}

// global slot

void QSetting::hasChanged() {
   save->setEnabled(true);
   restore->setEnabled(true);
}
