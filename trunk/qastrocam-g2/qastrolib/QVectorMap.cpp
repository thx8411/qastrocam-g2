#include "QVectorMap.moc"
#include <math.h>
#include <qpen.h>
#include <qpainter.h>
#include <qpixmap.h>

#include "Vector2D.hpp"

QVectorMap::QVectorMap(QWidget * parent, const char * name, WFlags f):
   QWidget(parent,name,f) {
   setWFlags(WRepaintNoErase);
   mode_=DrawPoint;
   add(Vector2D(0,0));
   scale_=1;
}

QVectorMap::~QVectorMap() {
}

void QVectorMap::setMode(DrawMode mode) {
   mode_=mode;
}

void QVectorMap::setScale(int scale) {
   scale_=scale;
   update();
}

void QVectorMap::reset() {
   vectorList_.clear();
   add(Vector2D(0,0));
   update();
}

void QVectorMap::add(const Vector2D & vect) {
   vectorList_.push_back(vect);
   update();
}

void QVectorMap::paintEvent(QPaintEvent * ev) {
   int w=size().width();
   int h=size().height();

   QPen dotPen;
   QPixmap buffer(size());
   QPainter p;
   buffer.fill(Qt::black); // erasing buffer
   p.begin(&buffer,this);
   // drawing scale
   if (scale_ != 1) {
      int fix=80;
      int greyLevel=fix+(255-fix)*scale_/100;
      dotPen.setColor(QColor(greyLevel,greyLevel,greyLevel));
      p.setPen(dotPen);
      for(int i=-w/scale_;
          i<w/scale_;
          ++i) {
         for(int j=-h/scale_;
             j<h/scale_;
             ++j) {
            p.drawPoint(i*scale_+w/2,h-(j*scale_+h/2));
         }
      }
   }
   if (!vectorList_.empty()) {
      VectorList::const_iterator it=vectorList_.begin();
      Vector2D previous=*it;
      
      int NbPoint=vectorList_.size();
      int curPoint=1;
      while(it != vectorList_.end()) {
         dotPen.setColor(QColor(255-(255*curPoint/NbPoint),
                                255*curPoint/NbPoint,
                                0));
         p.setPen(dotPen);
         switch (mode_) {
         case DrawPoint:
            p.drawPoint((int)round((*it).x()*scale_+w/2),
                        (int)round(h-((*it).y()*scale_+h/2)));
            break;
         case DrawLine:
            p.drawLine((int)round(previous.x()*scale_+w/2),
                       (int)round(h-(previous.y()*scale_+h/2)),
                       (int)round((*it).x()*scale_+w/2),
                       (int)round(h-((*it).y()*scale_+h/2)));
            break;
         }
         previous=*it;
         ++curPoint;
         ++it;
      }
   }
   p.end();
   bitBlt(this, 0, 0, &buffer);
}
