#include "QGridBox.moc"
#include <iostream>

using namespace std;

QGridBoxLayout::QGridBoxLayout(QWidget * parent , Orientation ori, int size,
                               const char * name) :
   QGridLayout(parent,(ori==Vertical)?size:1, (ori==Vertical)?1:size , /*margin*/ 0, /* space*/ -1, name),
   size_(size),
   nbElements_(0),
   orientation_(ori) {
   setAutoAdd(true);
}

void QGridBoxLayout::addItem(QLayoutItem * item ) {
   int col,row;
   if (orientation_==Vertical) {
      col=nbElements_%size_;
      row=nbElements_/size_;
   } else {
      row=nbElements_%size_;
      col=nbElements_/size_;
   }
   ++nbElements_;
   QGridLayout::addItem(item, row, col);
}

QLayoutIterator QGridBoxLayout::iterator () {
   return QGridLayout::iterator();
}

QGridBox::QGridBox(QWidget * parent , Orientation ori, int size,
                   const char * name) :
   QWidget(parent,name),
   layout_(this, ori, size , name) {
}
