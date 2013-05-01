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

#ifndef _QCAMVBOX_HPP_
#define _QCAMVBOX_HPP_

#include <Qt/qwidget.h>
#include <Qt/qboxlayout.h>
#include <QtCore/qcoreevent.h>
#include <QtGui/qevent.h>

//
// Replacement for Q3VBox
//

// there no 'window closed' signal in QWidget, we have to
// do it ourself, handling events

class QCamVBox: public QWidget {
   Q_OBJECT
   public:
      QCamVBox(QWidget* parent = NULL);
      ~QCamVBox();
      // for children detection
      bool event(QEvent *event);
   signals:
      void windowClosed();
   protected:
      void closeEvent(QCloseEvent *event);
   private:
      QVBoxLayout* widgetLayout;
};

#endif
