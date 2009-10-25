#include "FrameMirror.moc"
#include <qhbox.h>
#include <qpushbutton.h>

bool FrameMirror::transform(const QCamFrame in, QCamFrame & out) {
   if (in.empty()) {
      return false;
   }
   if (swapUpDown_ || swapLeftRight_ ||  true) {
      out.setMode(in.getMode());
      out.setSize(in.size());
      out.copy(in,
               0,0,
               in.size().width()-1,in.size().height()-1,
               0,0,
               swapLeftRight_,swapUpDown_);
   } else {
      out=in;
   }
   return true;
}

void FrameMirror::memswap(unsigned char *dest,
                          const unsigned char *src, size_t n) {
   unsigned i=0,j=n-1;
   while (i<n) {
      dest[i++]=src[j--];
   }
}

FrameMirror::FrameMirror() {
   swapUpDown_=swapLeftRight_=false;
}

void FrameMirror::swapLeftRight(bool val) {
   swapLeftRight_=val;
   emit(leftRightSwapped(val));
}

void FrameMirror::swapUpDown(bool val) {
   swapUpDown_=val;
   emit(upDownSwapped(val));
}

FrameMirror::Widget::Widget(QWidget * parent,const FrameMirror * algo):
   QHBox(parent) {
   upDown_ = new QPushButton(tr("swap Up/Down"),this);
   upDown_->setToggleButton(true);
   connect(upDown_,SIGNAL(toggled(bool)),algo,SLOT(swapUpDown(bool)));
   connect(algo,SIGNAL(upDownSwapped(bool)),upDown_,SLOT(setOn(bool)));

   leftRight_ = new QPushButton(tr("swap Left/Right"),this);
   leftRight_->setToggleButton(true);
   connect(leftRight_,SIGNAL(toggled(bool)),algo,SLOT(swapLeftRight(bool)));
   connect(algo,SIGNAL(leftRightSwapped(bool)),leftRight_,SLOT(setOn(bool)));
}

FrameMirror::Widget::~Widget() {
   delete leftRight_;
   leftRight_=NULL;
   delete upDown_;
   upDown_=NULL;
}
