#ifndef _FrameMirror_hpp_
#define _FrameMirror_hpp_

#include <qobject.h>
#include <qstring.h>
#include <qhbox.h>

#include "FrameAlgo.hpp"
class QPushButton;

class FrameMirror :  public FrameAlgo {
   Q_OBJECT;
private:
   class Widget : public QHBox {
   public:
      ~Widget();
      Widget(QWidget * parent,const FrameMirror * algo);
   private:
      QPushButton * upDown_;
      QPushButton * leftRight_;
   };

public:
   FrameMirror();
   bool transform(const QCamFrame in, QCamFrame & out);
   QString label() const {return "Mirror";}
   QWidget * allocGui(QWidget * parent) const {
      return new Widget(parent,this);
   }
public slots:
   void swapUpDown(bool);
   void swapLeftRight(bool);
signals:
   void upDownSwapped(bool);
   void leftRightSwapped(bool);
private:
   void memswap(unsigned char *dest,
                const unsigned char *src, size_t n);
   bool swapUpDown_;
   bool swapLeftRight_;
};


#endif
