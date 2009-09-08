#ifndef _QGridBox_h_
#define _QGridBox_h_

#include <qlayout.h>
#include <qwidget.h>

class QGridBoxLayout : public QGridLayout {
public:
   QGridBoxLayout(QWidget * parent , Orientation, int size,
                  const char * name = 0 );
   virtual void addItem ( QLayoutItem * item );
   virtual QLayoutIterator iterator ();
private:
   /// max width or height (depending of the Orientation
   const int size_;
   /// current number of inserted elements
   int nbElements_;
   /// orientation
   const Orientation orientation_;
};

class QGridBox : public QWidget {
   Q_OBJECT;
public:
   QGridBox(QWidget * parent , Orientation, int size,
            const char * name = 0 );
   // the grid layout
   QGridBoxLayout layout_;
};

#endif
