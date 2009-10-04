#include "QCamRadioBox.moc"
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
   valueList_=new int[numOfbutton];
   buttonTable_=new QRadioButton*[numOfbutton];
   numOfButton_=numOfbutton;
   rowTable_=new QHBox*[numOfbutton/maxPerRow+1];
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
   delete buttonTable_;
   delete valueList_;
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
