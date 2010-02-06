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

//
// TO BE REWROTE !!!
//


#include "QFileChooser.moc"

#include <qfiledialog.h>
#include <qtooltip.h>

//#include <libgen.h> // for basename
#include <stdlib.h>
//#include <unistd.h> // for get_current_dir_name

#include <iostream>

#include "QCamUtilities.hpp"

using namespace std;

QFileChooser::QFileChooser(QWidget * parent):
   QPushButton(parent) {
   QPixmap* tmpIcon;
   char * curFile=get_current_dir_name();
   connect(this,SIGNAL(pressed()),this,SLOT(selectFile()));
   setFile(curFile);
   free(curFile);
   tmpIcon=QCamUtilities::getIcon("choose_directory.png");
   setPixmap(*tmpIcon);
   delete tmpIcon;
}

QFileChooser::~QFileChooser() {
}

void QFileChooser::selectFile() {
   setDisabled(true);
   QString newDir = QFileDialog::getExistingDirectory(
      currentDir_,
      this,
      "get existing directory",
      "Choose a directory",
      TRUE );
   setDisabled(false);
   if (!newDir.isEmpty() && !newDir.isNull()) {
      setFile(newDir);
   }
}

void QFileChooser::setFile(const QString & fileName) {
   currentDir_=fileName;
   char * tmpStr=strdup(fileName.latin1());
   //setText(basename(tmpStr));
   free(tmpStr);
   //QToolTip::add(this, currentDir_);
   emit(fileChanged(fileName));
}
