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
   QVBox* remoteCTRL_;
   QString label_;
   QHGroupBox* videoBox;
   QHGroupBox* telescopeBox;
   QHGroupBox* lxBox;
   QVGroupBox* optionsBox;
   QHBox* buttonsBox;
   QPushButton* save;
   QPushButton* restore;
};

#endif
