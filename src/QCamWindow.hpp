/******************************************************************
Qastrocam-g2
Copyright (C) 2013   Blaise-Florentin Collin

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

#ifndef _QCAMWINDOW_HPP_
#define _QCAMWINDOW_HPP_

#include <Qt/qwidget.h>
#include <QtCore/qcoreevent.h>
#include <QtGui/qevent.h>

class QCamWindow : public QWidget {
   Q_OBJECT
   public:
      QCamWindow(QWidget* parent = NULL, const char* name = NULL, Qt::WFlags f = 0);
      ~QCamWindow();
   signals:
      void windowClosed();
   protected:
      void closeEvent(QCloseEvent *event);
};

#endif
