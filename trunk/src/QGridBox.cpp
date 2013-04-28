/******************************************************************
Qastrocam
Copyright (C) 2003-2009   Franck Sicard
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

#include <iostream>

#include "QGridBox.hpp"

using namespace std;

void QGridBox::addWidget(QWidget* w) {
   int col,row;
   if (orientation_==Qt::Vertical) {
      col=nbElements_%size_;
      row=nbElements_/size_;
   } else {
      row=nbElements_%size_;
      col=nbElements_/size_;
   }
   ++nbElements_;
   layout_.addWidget(w, row, col);
}

QGridBox::QGridBox(QWidget* parent , Qt::Orientation ori, int size, const char* name) : QWidget(parent),
      size_(size), nbElements_(0), orientation_(ori) {

   // setting margin and spacing
   layout_.setContentsMargins(0,0,0,0);
   layout_.setSpacing(0);

   setLayout(&layout_);
}

// handling children widget add/supp
bool QGridBox::event(QEvent *event)
{
      // if child event
      if (event->type() == QEvent::ChildAdded) {
         // casting event
         QChildEvent *childEvent=static_cast<QChildEvent*>(event);
         // if this child is a widget
         if(childEvent->child()->isWidgetType()) {
            // cast to a widget
            QWidget* childObject=static_cast<QWidget*>(childEvent->child());
            // add or remove the widget
            if(childEvent->added())
               addWidget(childObject);
         }
      }
      // call the base event handler
      return QWidget::event(event);
}
