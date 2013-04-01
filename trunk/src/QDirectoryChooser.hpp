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


#ifndef _QDirectoryChooser_hpp_
#define _QDirectoryChooser_hpp_

#include <qpushbutton.h>
#include <qobject.h>

class QDirectoryChooser : public QPushButton {
   Q_OBJECT
   QString currentDir_;
public:
   QDirectoryChooser(QWidget * parent=0);
   ~QDirectoryChooser();
private slots:
   void selectDirectory();
   void setDirectory(const QString &);
signals:
   void directoryChanged(const QString &);
};

#endif
