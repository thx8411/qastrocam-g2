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

//
// Replacement for Q3HGroupBox
//

// there no 'window closed' signal in QWidget, we have to
// do it ourself, handling events


#include "QCamHGroupBox.hpp"

// constructor
QCamHGroupBox::QCamHGroupBox(const QString& title, QWidget* parent):QGroupBox(title, parent) {
   widgetLayout=new QHBoxLayout();
   setLayout(widgetLayout);
}

// destructor
QCamHGroupBox::~QCamHGroupBox() {
   delete widgetLayout;
}

// handling children widget add/supp
bool QCamHGroupBox::event(QEvent *event)
{
      // if child event
      if ((event->type() == QEvent::ChildAdded)||(event->type() == QEvent::ChildRemoved)) {
         // casting event
         QChildEvent *childEvent=static_cast<QChildEvent*>(event);
         // if this child is a widget
         if(childEvent->child()->isWidgetType()) {
            // cast to a widget
            QWidget* childObject=static_cast<QWidget*>(childEvent->child());
            // add or remove the widget
            if(childEvent->added())
               widgetLayout->addWidget(childObject);
            else if(childEvent->removed())
               widgetLayout->removeWidget(childObject);
         }
      }
      // call the base event handler
      return QGroupBox::event(event);
}

// close event caption
void QCamHGroupBox::closeEvent(QCloseEvent *event)
{
   emit(windowClosed());
   event->accept();
}
