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


#ifndef _QSETTING_HPP_
#define _QSETTING_HPP_

#include <qobject.h>
#include <qwidget.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qhgroupbox.h>
#include <qvgroupbox.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qlabel.h>

#include "QDirectoryChooser.hpp"
#include "QFileChooser.hpp"
#include "QCamComboBox.hpp"

// settings gui for qastrocam-g2

class QSetting : public QObject {
   Q_OBJECT
public :
   QSetting();
   ~QSetting();
   virtual QWidget* buildGUI(QWidget * parent);
   const QString & label() const;
public slots:
   // buttons
   void saveSettings();
   void restoreSettings();
   // entry boxs
   void changeVideoDevice(const QString& name);
   void changeTelescopeDevice(const QString& name);
   void changeLxDevice(const QString& name);
   void changeLibpath(const QString& name);
   // combobox
   void changeTelescope(int index);
   void changeCamera(int index);
   // global
   void hasChanged();
   // checkbox
   void setNightVision(bool state);
private :
   // functions
   void fillFields();
   // gui
   QVBox* remoteCTRL_;
   QString label_;
   QHGroupBox* videoBox;
   QLabel* camLabel;
   QCamComboBox* cameraList;
   QLabel* videoDeviceLabel;
   QLineEdit* videoDeviceEntry;
   QFileChooser* videoDeviceChooser;
   QVGroupBox* telescopeBox;
   QHBox* lineOne;
   QHBox* lineTwo;
   QLabel* telescopeListLabel;
   QCamComboBox* telescopeList;
   QLabel* telescopeDeviceLabel;
   QLineEdit* telescopeDeviceEntry;
   QFileChooser* telescopeDeviceChooser;
   int telescopeDeviceType;
   QWidget* padding0;
   QWidget* padding1;
   QWidget* padding2;
   QWidget* padding3;
   QWidget* padding4;
   QWidget* padding5;
   QWidget* padding6;
   QWidget* padding7;
   QWidget* padding8;
   QWidget* padding9;
   QCheckBox* telescopeLevels;
   QHGroupBox* lxBox;
   QLabel* lxDeviceLabel;
   QLineEdit* lxDeviceEntry;
   QFileChooser* lxDeviceChooser;
   QCheckBox* lxLevels;
   QVGroupBox* modulesBox;
   QHBox* lineFive;
   QHBox* lineSix;
   QCheckBox* modulesAdd;
   QCheckBox* modulesMax;
   QCheckBox* modulesKing;
   QCheckBox* modulesAlign;
   QVGroupBox* optionsBox;
   QHBox* lineThree;
   QHBox* lineFour;
   QHBox* lineSeven;
   QCheckBox* optionsSdl;
   QCheckBox* optionsExpert;
   QCheckBox* optionsLog;
   QCheckBox* optionsNightVision;
   QCheckBox* optionsRegistax;
   QHBox* libBox;
   QLabel* libpathLabel;
   QLineEdit* libpathEntry;
   QDirectoryChooser* libpathChooser;
   QHBox* buttonsBox;
   QPushButton* save;
   QPushButton* restore;
};

#endif
