#ifndef _QBLINK_HPP_
#define _QBLINK_HPP_

#include <qlabel.h>

// very simple class showing some activity
// eatch time you call the "step" member
// (one char rotating indicator, - \ | /)

class QBlink : public QLabel {
   Q_OBJECT
public :
   QBlink(QWidget* parent);
   void step();
private :
   int position;
   QString items[4];
};

#endif
