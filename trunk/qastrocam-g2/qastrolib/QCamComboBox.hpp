/******************************************************************
Qastrocam
Copyright (C) 2003-2009   Franck Sicard
Qastrocam-g2
Copyright (C) 2009   Blaise-Florentin Collin

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License v2
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
MA  02110-1301, USA.
*******************************************************************/


#ifndef _QCamComboBox_hpp_
#define _QCamComboBox_hpp_

#include <qcombobox.h>

/** Handles a QComboBox like a QCamRadioBox.
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
   int getPosition(const char* item);
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
