#ifndef _QFrameDisplay_hpp_
#define _QFrameDisplay_hpp_

#include <qwidget.h>
#include "QCamFrame.hpp"

class QPainter;

class QPen;

/** Simple widget that can display a QCamFrame */
class QFrameDisplay : public QWidget {
public:
   QFrameDisplay(QWidget * parent,const char * label);
   void frame(const QCamFrame &);
protected:
   void paintEvent(QPaintEvent * ev);
private:
   QCamFrame frame_;
   QPainter * painter_;
   QPen * pen_;
};
#endif
