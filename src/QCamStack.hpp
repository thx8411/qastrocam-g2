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


#ifndef _QCAMSTACK_HPP_
#define _QCAMSTACK_HPP_

#include <qobject.h>
#include <qstring.h>
#include <qwidget.h>
#include <qvbox.h>
#include <qhgroupbox.h>

#include "QCam.hpp"

#define CAMSTACK_SIZE	8

// for stacking cams
class QCamStack : public QObject {
   Q_OBJECT
public :
   QCamStack();
   ~QCamStack();
   QWidget* buildGUI(QWidget * parent);
   const QString & label() const;
   // add a cam with the given name in the stack
   void addCam(QCam* cam, QString name);
public slots:
private :
   QVBox* remoteCTRL_;
   QString label_;
   // stack
   // name list
   QString nameTab[CAMSTACK_SIZE];
   // cam list
   QCam* camTab[CAMSTACK_SIZE];
   // group box list
   QHGroupBox* groupTab[CAMSTACK_SIZE];
   // last +1 cam index
   int camIndex;
};

#endif
