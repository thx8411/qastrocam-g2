#ifndef _QCamRadioBox_hpp_
#define _QCamRadioBox_hpp_

#include <qvgroupbox.h>

class QRadioButton;
class QVGroupBox;
class QButtonGroup;
class QHBox;

/** Handle some radiobox buttons in a
    QButtonGroup.
*/
class QCamRadioBox: public QVGroupBox {
   Q_OBJECT
public:
   QCamRadioBox(const char * label /** label of the box */,
                QWidget * parent,
                int numOfbutton /** num of buttons */,
                int valueList[] /** value assossiated to the button by signal change() */,
                const char *  labelList[]=NULL /** labels of the buttons */,
                int maxPerRow=100 /** max number of checkbox by row */);
   ~QCamRadioBox();
   int value() const { return currentValue_; }
public slots:
  /** set the state of the radiobox.
      The given value must be one of the valueList.
  */
  void update(int value);

signals:
   /** send when a checkbox is checked.
    */
   void change(int value);
protected slots:
   void buttonClicked(int id);
private:
   QButtonGroup *bg_;
   QHBox ** rowTable_;
   QRadioButton **  buttonTable_;
   int * valueList_;
   int numOfButton_;
   int currentValue_;
};

#endif
