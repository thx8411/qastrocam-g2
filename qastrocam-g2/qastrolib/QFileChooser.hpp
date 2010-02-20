/******************************************************************
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


#ifndef _QFileChooser_hpp_
#define _QFileChooser_hpp_

#include <qpushbutton.h>
#include <qobject.h>

// files types :
#define	DEVICE_FILE	0
#define REGULAR_FILE	1
#define IMAGE_FILE	2

class QFileChooser : public QPushButton {
   Q_OBJECT;
   QString currentDir_;
public:
   QFileChooser(QWidget * parent=0,int type=1);
   ~QFileChooser();
   void setType(int type);
private:
   int fileType;
private slots:
   void selectFile();
   void setFile(const QString &);
signals:
   void fileChanged(const QString &);
};

#endif
