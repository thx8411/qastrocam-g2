#ifndef _QCamComboBox_hpp_
#define _QCamComboBox_hpp_

#include <qcombobox.h>

/** Handle a QComboBox like a QCamRadioBox.
*/

class QCamComboBox: public QComboBox {
   Q_OBJECT
public:
   QCamComboBox(const char * label /** label of the box */,
                QWidget * parent,
                int numOfbutton /** num of buttons */,
                int valueList[] /** value assossiated to the button bye signal change() */,
                const char *  labelList[]=NULL /** labels of the buttons */);
   ~QCamComboBox();
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
   int * valueList_;
   int numOfButton_;
   int currentValue_;
};

#endif
