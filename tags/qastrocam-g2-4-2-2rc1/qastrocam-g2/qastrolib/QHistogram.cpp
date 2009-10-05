#include "QHistogram.moc"
#include <string.h>
#include <iostream>
#include <qpen.h>
#include <qpainter.h>
#include <qpixmap.h>

#include <math.h>

using namespace std;

#define DOUBLE_MIN (-1e38)
#define DOUBLE_MAX 1e38

QHistogram::QHistogram(QWidget * parent, const char * name, WFlags f):
   QWidget(parent,name,f) {
   dataSize_=0;
   dataTable_=NULL;
   currentPos_=0;
   autoShift_=false;
   average_=0;
   normPen_=new QPen();
   normPen_->setColor(QColor(0,0,0));
   averagePen_=new QPen();
   averagePen_->setColor(QColor(255,0,0));
   max_=DOUBLE_MIN;
   min_=DOUBLE_MAX;
   setWFlags(WRepaintNoErase);
   dispMode_=NormalDisplay;
   setDataSize(20);
}

QHistogram::~QHistogram() {
   delete dataTable_;
   delete normPen_;
   delete averagePen_;
}

void QHistogram::reset() {
   for (int i=0;i<dataSize_;++i) dataTable_[i]=0.0;
   max_=DOUBLE_MIN;
   min_=DOUBLE_MAX;
}

void QHistogram::setDataSize(int size) {
   dataSize_=currentPos_=0;
   delete dataTable_;
   dataTable_= new double[size];
   dataSize_=size;
   reset();
}

void QHistogram::setValue(double value,int pos) {
   if (pos>dataSize_) {
      cerr << "index "<<pos<<" out of range in QHistogram::addValue()"<<endl;
      return;
   }
   if (autoShift()) {
      --currentPos_;
      if (currentPos_<0) currentPos_+=dataSize_;
      //cout << "insert " << value << " at " <<  currentPos_+pos <<endl;
   }
   pos=(currentPos_+pos)%dataSize_;
   
   bool searchMax=dataTable_[pos]==max_;
   bool searchMin=dataTable_[pos]==min_;
   
   dataTable_[pos]=value;
   
   if (value>max_) max_=value;
   if (value<min_) min_=value;
   
   if (searchMax) max_=findMax();
   if (searchMin) min_=findMin();

   if (autoShift()) draw();
}

double QHistogram::min() const { return conv2displayMode(min_);}

double QHistogram::max() const { return conv2displayMode(max_);}

double QHistogram::findMax() const {
   double max=dataTable_[0];
   for (int i=1;i<dataSize_;++i) {
      if (dataTable_[i]>max) max=dataTable_[i];
   }
   return max;
}

double QHistogram::findMin() const {
   double min=dataTable_[0];
   for (int i=1;i<dataSize_;++i) {
      if (dataTable_[i]<min) min=dataTable_[i];
   }
   return min;
}

bool QHistogram::autoShift() const {
   return autoShift_;
}

int QHistogram::average() const {
   return average_;
}

void QHistogram::setAverage(int val) {
   average_=val;
}

void QHistogram::setAutoShift(bool val) {
   autoShift_=val;
}

double QHistogram::value(int pos) const {
   if (pos>=dataSize_) {
      cerr << "index "<<pos<<" out of range in QHistogram::value()"<<endl;
      return 0;
   }
   return conv2displayMode(dataTable_[(currentPos_+pos)%dataSize_]);
}

double QHistogram::value(int pos1,int pos2) const {
   double sum=0;
   for(int i=pos1;i<=pos2;++i) {
      sum+=value(i);
   }
   sum/=(pos2-pos1+1);
   return sum;
}

void QHistogram::paintEvent(QPaintEvent * ev) {
   int w=size().width();
   int h=size().height();

   int pivot=0;
   int reverse=1;

   if (autoShift()) {
      pivot=w-1;
      reverse=-1;
   }
   QPixmap buffer(size());
   QPainter p;
   buffer.fill();
   p.begin(&buffer,this);
   p.setPen(*normPen_);
   for(int i=0;i<dataSize_;++i) {
      p.drawLine(pivot +reverse*i*w/dataSize_,
                 h-1,
                 pivot +reverse*i*w/dataSize_,
                 h-1-((int)round(value(i)*h/max())));
   }
   p.setPen(*averagePen_);
   if (average()) {
      for(int i=average();i<dataSize_-average()-1;++i) {
         double first,second;
         first=value(i-average(),i+average());
         second=value(i-average()+1,i+average()+1);
         p.drawLine(pivot +reverse*i*w/dataSize_,h-1-(int)(first*h/max()),
                    pivot +reverse*(i+1)*w/dataSize_,h-1-(int)(second*h/max()));
      }
   }
   p.end();
   bitBlt(this, 0, 0, &buffer);
}

void QHistogram::displayMode(DisplayMode mode) {
   dispMode_=mode;
}

double QHistogram::conv2displayMode(double val) const {
   switch(dispMode_) {
   case NormalDisplay:
      return val;
   case SqrtDisplay:
      return sqrt(val);
   case LogDisplay:
      return log(val+1);
   case Power2Display:
      return val*val;
   case ExpDisplay:
      return exp(val);
   default:
      cerr << "invalid display mode "<<dispMode_<<" in QHistogram::conv2displayMode"<<endl;
      return val;
   }
}