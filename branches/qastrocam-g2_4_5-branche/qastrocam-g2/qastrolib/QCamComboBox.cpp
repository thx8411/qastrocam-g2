#include "QCamComboBox.moc"
#include <qradiobutton.h>
#include <qhbox.h>
#include <qbuttongroup.h>
#include <iostream>
#include <string.h>

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

int QCamComboBox::getPosition(const char* item) {
   for(int i=0; i<numOfButton_;i++)
	if(QString(item)==text(i)) return(i);
   return(-1);
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
