#include "QCamComboBox.moc"
#include <qradiobutton.h>
#include <qhbox.h>
#include <qbuttongroup.h>
#include <iostream>

using namespace std;

QCamComboBox::QCamComboBox(const char * label,QWidget * parent,
                           int numOfbutton, int valueList[],
                           const char * labelList[]):
   QComboBox(false,parent,label) {
   valueList_=new int[numOfbutton];
   numOfButton_=numOfbutton;
      
   for (int i=0;i<numOfButton_;++i) {
      valueList_[i]=valueList[i];
      insertItem(labelList?QString(labelList[i]):QString("%1").arg(valueList_[i]));
   }
   connect(this,SIGNAL(activated(int)),this,SLOT(buttonClicked(int)));
   currentValue_=valueList[0];
   update(currentValue_);
}

QCamComboBox::~QCamComboBox() {
   delete valueList_;
}


void QCamComboBox::update(int value) {
   for (int i=0;i<numOfButton_;++i) {
      if (valueList_[i]==value) {
         setCurrentItem(i);
         currentValue_=value;
         //emit(change(value));
         return;
      }
   }
   cout << "invalid value "<<value
        <<" for widget QCamComboBox::"
        << name()<<endl;
}

void QCamComboBox::buttonClicked(int id) {
   emit(change(currentValue_=valueList_[id]));
};
