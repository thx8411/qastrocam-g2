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

QGridBoxLayout::QGridBoxLayout(QWidget* parent , Qt::Orientation ori, int size,const char* name) :
   QGridLayout(parent,(ori==Qt::Vertical)?size:1, (ori==Qt::Vertical)?1:size , /*margin*/ 0, /* space*/ -1, name),
   size_(size),
   nbElements_(0),
   orientation_(ori) {
   setAutoAdd(true);
}

void QGridBoxLayout::addItem(QLayoutItem * item ) {
   int col,row;
   if (orientation_==Qt::Vertical) {
      col=nbElements_%size_;
      row=nbElements_/size_;
   } else {
      row=nbElements_%size_;
      col=nbElements_/size_;
   }
   ++nbElements_;
   QGridLayout::addItem(item, row, col);
}

QGridBox::QGridBox(QWidget* parent , Qt::Orientation ori, int size, const char* name) :
   QWidget(parent,name),
   layout_(this, ori, size , name) {
}
