#include "QFrameDisplay.hpp"
#include "QCamFrame.hpp"
#include <qcolor.h>
#include <qpen.h>
#include <qpainter.h>

QFrameDisplay::QFrameDisplay(QWidget * parent,const char * label) :
   QWidget(parent,label) {
   painter_ = new QPainter();
   pen_=new QPen();
   pen_->setStyle(DotLine);
   pen_->setColor(QColor(255,0,0));
   setWFlags(WRepaintNoErase);
}

void QFrameDisplay::frame(const QCamFrame &frame) {
   frame_=frame;
   setMinimumSize(frame_.size());
   resize(frame_.size());
   update();
}
   
void QFrameDisplay::paintEvent(QPaintEvent * ev) {
   if (!frame_.empty()) {
      painter_->begin(this);
      painter_->setPen(*pen_);
      painter_->setClipRegion(ev->region());
      painter_->drawImage(0,0,frame_.colorImage());
      painter_->drawLine(width()/2,0,
                         width()/2,height());
      painter_->drawLine(0,height()/2,
                         width(),height()/2);
      painter_->end();
   }
}
