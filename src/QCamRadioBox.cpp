/******************************************************************
Qastrocam
Copyright (C) 2003-2009   Franck Sicard
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

#include <stdlib.h>

#include "QCamRadioBox.hpp"
#include <qradiobutton.h>
#include <qhbox.h>
#include <qbuttongroup.h>
#include <iostream>

using namespace std;

QCamRadioBox::QCamRadioBox(const char * label,QWidget * parent,
                           int numOfbutton, int valueList[],
                           const char * labelList[],
                           int maxPerRow ):
   QVGroupBox(label,parent) {
   valueList_=(int*)malloc(numOfbutton*sizeof(int));
   buttonTable_=(QRadioButton**)malloc(sizeof(QRadioButton*)*numOfbutton);
   numOfButton_=numOfbutton;
   rowTable_=(QHBox**)malloc((numOfbutton/maxPerRow+1)*sizeof(QHBox*));
   for (int i=0;i<=numOfbutton/maxPerRow;++i) {
      rowTable_[i]=new QHBox(this);
   }
   bg_=new QButtonGroup(this);
   bg_->hide();
   for (int i=0;i<numOfButton_;++i) {
      valueList_[i]=valueList[i];
      buttonTable_[i]
         =new QRadioButton(labelList?QString(labelList[i]):QString("%1").arg(valueList_[i]),
                           rowTable_[i/maxPerRow]);
      buttonTable_[i]->show();
      bg_->insert(buttonTable_[i]);
   }
   for (int i=0;i<=numOfbutton/maxPerRow;++i) {
      rowTable_[i]->show();
   }
   connect(bg_,SIGNAL(clicked(int)),this,SLOT(buttonClicked(int)));
   currentValue_=valueList[0];
   //update(currentValue_);
}

QCamRadioBox::~QCamRadioBox() {
   while (numOfButton_-- > 0) {
      buttonTable_[numOfButton_]->hide();
      delete buttonTable_[numOfButton_];
   }
   free(buttonTable_);
   free(valueList_);
   free(rowTable_);
}


void QCamRadioBox::update(int value) {
   for (int i=0;i<numOfButton_;++i) {
      if (valueList_[i]==value) {
         buttonTable_[i]->setChecked(true);
         currentValue_=value;
         //emit(change(value));
         return;
      }
   }
   cout << "invalid value "<<value
        <<" for widget QCamRadioBox::"
        << name()<<endl;
}

void QCamRadioBox::buttonClicked(int id) {
   emit(change(currentValue_=valueList_[id]));
};
