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


#ifndef _QSETTING_HPP_
#define _QSETTING_HPP_

#include <qobject.h>
#include <qwidget.h>
#include <qhbox.h>
#include <qhgroupbox.h>
#include <qvgroupbox.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qwidget.h>

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
   void saveSettings();
   void restoreSettings();
private :
   // vars
   bool hasChanged;
   // gui
   QVBox* remoteCTRL_;
   QString label_;
   QHGroupBox* videoBox;
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
   QWidget* padding0;
   QWidget* padding1;
   QWidget* padding2;
   QWidget* padding3;
   QWidget* padding4;
   QWidget* padding5;
   QWidget* padding6;
   QWidget* padding7;
   QWidget* padding8;
   QCheckBox* telescopeLevels;
   QHGroupBox* lxBox;
   QLabel* lxDeviceLabel;
   QLineEdit* lxDeviceEntry;
   QFileChooser* lxDeviceChooser;
   QCheckBox* lxLevels;
   QVGroupBox* optionsBox;
   QHBox* lineThree;
   QHBox* lineFour;
   QCheckBox* optionsSdl;
   QCheckBox* optionsExpert;
   QCheckBox* optionsLog;
   QCheckBox* optionsForceGeneric;
   QHBox* libBox;
   QLabel* libpathLabel;
   QLineEdit* libpathEntry;
   QDirectoryChooser* libpathChooser;
   QHBox* buttonsBox;
   QPushButton* save;
   QPushButton* restore;
};

#endif
