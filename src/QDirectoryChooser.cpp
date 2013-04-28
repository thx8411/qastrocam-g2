/******************************************************************
Qastrocam
Copyright (C) 2003-2009   Franck Sicard
Qastrocam-g2
Copyright (C) 2009-2013   Blaise-Florentin Collin

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

#include <libgen.h> // for basename
#include <stdlib.h>
#include <unistd.h> // for get_current_dir_name

#include <iostream>

#include "QDirectoryChooser.hpp"

#include <Qt/qfiledialog.h>
#include <Qt/qtooltip.h>
#include <Qt/qpixmap.h>

#include "QCamUtilities.hpp"

using namespace std;

QDirectoryChooser::QDirectoryChooser(QWidget * parent):
   QPushButton(parent) {
   QIcon* tmpIcon;
   char * curDir=get_current_dir_name();
   connect(this,SIGNAL(pressed()),this,SLOT(selectDirectory()));
   setDirectory(curDir);
   free(curDir);
   tmpIcon=QCamUtilities::getIcon("choose_directory.png");
   if(tmpIcon!=NULL) {
      setIcon(*tmpIcon);
      delete tmpIcon;
   } else
      setText("Path...");
}

QDirectoryChooser::~QDirectoryChooser() {
}

void QDirectoryChooser::selectDirectory() {
   setDisabled(true);
   QString newDir = QFileDialog::getExistingDirectory(this, "Choose a directory", currentDir_);
   setDisabled(false);
   if (!newDir.isEmpty() && !newDir.isNull()) {
      setDirectory(newDir);
   }
}

void QDirectoryChooser::setDirectory(const QString & dirName) {
   currentDir_=dirName;
   char * tmpStr=strdup(dirName.toLatin1());
   //setText(basename(tmpStr));
   free(tmpStr);
   setToolTip(currentDir_);
   emit(directoryChanged(dirName));
}
