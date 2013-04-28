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

#include <iostream>

#include <Qt/qradiobutton.h>

#include "QCamRadioBox.hpp"

using namespace std;

QCamRadioBox::QCamRadioBox(const char * label,QWidget * parent,int numOfbutton, int valueList[],
                           const char * labelList[],int maxPerRow ):QGroupBox(label,parent) {

   valueList_=(int*)malloc(numOfbutton*sizeof(int));
   buttonTable_=(QRadioButton**)malloc(sizeof(QRadioButton*)*numOfbutton);
   numOfButton_=numOfbutton;

   globalLayout_=new QVBoxLayout();
   this->setLayout(globalLayout_);

   // sub group
   bg_=new QGroupBox(this);
   globalLayout_->addWidget(bg_);
   rowTable_=new QGridLayout(bg_);

   // Filling with radio buttons
   for (int i=0;i<numOfButton_;++i) {
      valueList_[i]=valueList[i];
      buttonTable_[i]=new QRadioButton(labelList?QString(labelList[i]):QString("%1").arg(valueList_[i]),bg_);
      buttonTable_[i]->show();
      rowTable_->addWidget(buttonTable_[i],i/maxPerRow,i%maxPerRow);
   }

   // connecting radio buttons
   for(int i=0; i<numOfButton_; i++) {
      connect( buttonTable_[i],SIGNAL(toggled(bool)),this,SLOT(buttonClicked(bool)));
   }

   if(numOfButton_>0) {
      currentValue_=valueList[0];
      buttonTable_[0]->setChecked(true);
   }
}

QCamRadioBox::~QCamRadioBox() {
   // deleting radio buttons
   while (numOfButton_-- > 0) {
      buttonTable_[numOfButton_]->hide();
      delete buttonTable_[numOfButton_];
   }
   delete rowTable_;
   delete bg_;
   delete globalLayout_;

   // free tables
   free(buttonTable_);
   free(valueList_);
}

void QCamRadioBox::update(int value) {
   for (int i=0;i<numOfButton_;++i) {
      if (valueList_[i]==value) {
         buttonTable_[i]->setChecked(true);
         currentValue_=value;
         return;
      }
   }
   cout << "invalid value " << value << " for widget QCamRadioBox" << endl;
}

void QCamRadioBox::buttonClicked(bool d) {
   int id;

   for(int i=0; i<numOfButton_;i++) {
      if(buttonTable_[i]->isChecked())
         id=i;
   }
   emit(change(currentValue_=valueList_[id]));
};
