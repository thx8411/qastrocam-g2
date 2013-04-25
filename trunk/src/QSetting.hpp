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

#include <Qt/qobject.h>
#include <Qt/qwidget.h>
#include <Qt/qpushbutton.h>
#include <Qt/qlineedit.h>
#include <Qt/qcheckbox.h>
#include <Qt/qlayout.h>
#include <Qt/qlabel.h>

#include <Qt3Support/q3hbox.h>
#include <Qt3Support/q3vbox.h>
#include <Qt3Support/q3hgroupbox.h>

#include "QCamVGroupBox.hpp"
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
   Q3VBox* remoteCTRL_;
   QString label_;
   Q3HGroupBox* videoBox;
   QLabel* camLabel;
   QCamComboBox* cameraList;
   QLabel* videoDeviceLabel;
   QLineEdit* videoDeviceEntry;
   QFileChooser* videoDeviceChooser;
   QCamVGroupBox* telescopeBox;
   Q3HBox* lineOne;
   Q3HBox* lineTwo;
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
   Q3HGroupBox* lxBox;
   QLabel* lxDeviceLabel;
   QLineEdit* lxDeviceEntry;
   QFileChooser* lxDeviceChooser;
   QCheckBox* lxLevels;
   QCamVGroupBox* modulesBox;
   Q3HBox* lineFive;
   Q3HBox* lineSix;
   QCheckBox* modulesAdd;
   QCheckBox* modulesMax;
   QCheckBox* modulesKing;
   QCheckBox* modulesAlign;
   QCamVGroupBox* optionsBox;
   Q3HBox* lineThree;
   Q3HBox* lineFour;
   Q3HBox* lineSeven;
   QCheckBox* optionsSdl;
   QCheckBox* optionsExpert;
   QCheckBox* optionsLog;
   QCheckBox* optionsNightVision;
   QCheckBox* optionsRegistax;
   Q3HBox* libBox;
   QLabel* libpathLabel;
   QLineEdit* libpathEntry;
   QDirectoryChooser* libpathChooser;
   Q3HBox* buttonsBox;
   QPushButton* save;
   QPushButton* restore;
};

#endif
