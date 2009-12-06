#ifndef _QHistogram_hpp_
#define _QHistogram_hpp_

#include <qwidget.h>

class QPen;
class QPaintEvent;

/** display an histogram. */
class QHistogram : public QWidget {
   Q_OBJECT;
public:
   enum DisplayMode {NormalDisplay=0,LogDisplay=1,SqrtDisplay=2,
                     Power2Display=3,ExpDisplay=4};

   QHistogram(QWidget * parent=0, const char * name=0, WFlags f=0 );
   virtual ~QHistogram();
   // histogram is shifted after each added value
   bool autoShift() const;
   /** number of value/2+1 used to calcultate average.
       0 for no average drawing. */
   int average() const;
   double value(int pos=0) const;
   double value(int pos1,int pos2) const;
   double max() const;
   double min() const;
public slots:
   // number of element to plot;
   void setDataSize(int dataSize);
   void setValue(double value,int pos=0);
   void setAutoShift(bool);
   void setAverage(int val);
   void reset();
   void displayMode(DisplayMode);
   // should called when autoShift()==false, after finishing the graph update;
   void draw() { update(); }
protected:
   void paintEvent( QPaintEvent * ev);
private:
   double findMax() const;
   double findMin() const;
   double conv2displayMode(double val) const;
   DisplayMode dispMode_;
   double max_;
   double min_;
   int dataSize_;
   double * dataTable_;
   int currentPos_;
   bool autoShift_;
   int average_;
   QPen * normPen_;
   QPen * averagePen_;
};

#endif
