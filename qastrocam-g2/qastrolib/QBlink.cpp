#include "QBlink.moc"

QBlink::QBlink(QWidget* parent) : QLabel(parent) {
   position=0;
   items[0]="-";
   items[1]="\\";
   items[2]="|";
   items[3]="/";
   setText(items[position]);
}

void QBlink::step(){
   position++;
   position%=4;
   setText(items[position]);
}
