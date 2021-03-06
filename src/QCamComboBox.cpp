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

#include <stdlib.h>
#include <string.h>

#include <iostream>

#include "QCamComboBox.hpp"

using namespace std;

QCamComboBox::QCamComboBox(const char* label,QWidget* parent, int numOfbutton, int valueList[],
                           const char* labelList[]):QComboBox(parent) {

   valueList_=(int*)malloc(numOfbutton*sizeof(int));
   numOfButton_=numOfbutton;

   for (int i=0;i<numOfButton_;++i) {
      valueList_[i]=valueList[i];
      addItem(labelList?QString(labelList[i]):QString("%1").arg(valueList_[i]));
   }
   connect(this,SIGNAL(activated(int)),this,SLOT(buttonClicked(int)));
   currentValue_=valueList[0];
   update(currentValue_);
}

QCamComboBox::~QCamComboBox() {
   free(valueList_);
}

int QCamComboBox::getPosition(const char* item) {
   for(int i=0; i<numOfButton_;i++)
	if(QString(item)==itemText(i)) return(i);
   return(-1);
}

void QCamComboBox::updateSignal(int value) {
   emit(change(value));
}

void QCamComboBox::update(int value) {
   for (int i=0;i<numOfButton_;++i) {
      if (valueList_[i]==value) {
         setCurrentIndex(i);
         currentValue_=value;
         //emit(change(value));
         return;
      }
   }
   cout << "invalid value " << value <<" for widget QCamComboBox" << endl;
}

void QCamComboBox::buttonClicked(int id) {
   emit(change(currentValue_=valueList_[id]));
};
