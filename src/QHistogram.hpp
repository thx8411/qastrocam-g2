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


#ifndef _QHistogram_hpp_
#define _QHistogram_hpp_

#include <Qt/qwidget.h>
//Added by qt3to4:
#include <QtGui/QPaintEvent>

class QPen;
class QPaintEvent;

/** display an histogram. */
class QHistogram : public QWidget {
   Q_OBJECT
public:
   enum DisplayMode {NormalDisplay=0,LogDisplay=1,SqrtDisplay=2,
                     Power2Display=3,ExpDisplay=4};

   QHistogram(QWidget * parent=0, const char * name=0, Qt::WFlags f=0 );
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
