#ifndef _QVectorMap_hpp_
#define _QVectorMap_hpp_

#include <qwidget.h>
#include <qvaluelist.h>
#include "Vector2D.hpp"
class QPaintEvent;

enum DrawMode {
   DrawPoint,
   DrawLine
};

class QVectorMap : public QWidget {
   Q_OBJECT;
public:
   QVectorMap(QWidget * parent=0, const char * name=0, WFlags f=0 );
   virtual ~QVectorMap();
 public slots:
   void add(const Vector2D&);
   void reset();
   void setMode(DrawMode);
   void setScale(int scale);
protected:
   void paintEvent( QPaintEvent * ev);
private:
   typedef QValueList<Vector2D> VectorList;
   VectorList vectorList_;
   DrawMode mode_;
   int scale_;
};

#endif
