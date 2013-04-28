/******************************************************************
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

#include <stdlib.h>
#include <unistd.h>

#include <iostream>

#include <Qt/qfiledialog.h>
#include <Qt/qpixmap.h>

#include "QFileChooser.hpp"
#include "QCamUtilities.hpp"

using namespace std;

QFileChooser::QFileChooser(QWidget * parent, int type):
   QPushButton(parent) {
   QIcon* tmpIcon;
   connect(this,SIGNAL(pressed()),this,SLOT(selectFile()));
   tmpIcon=QCamUtilities::getIcon("choose_directory.png");
   if(tmpIcon!=NULL) {
      setIcon(*tmpIcon);
      delete tmpIcon;
   } else
      setText("File...");
   fileType=type;
}

QFileChooser::~QFileChooser() {
}

void QFileChooser::setType(int type) {
   fileType=type;
}

void QFileChooser::selectFile() {
   setDisabled(true);
   QString newFile;

   // device dialog box
   if(fileType==DEVICE_FILE) {
      newFile=QFileDialog::getOpenFileName(this, "Choose a device", "/dev/", "Devices (*)");
   // file dialog box
   } else if(fileType==REGULAR_FILE) {
      newFile=QFileDialog::getSaveFileName(this, "Choose a file", get_current_dir_name(), "Files (*)");
   } else {
      newFile=QFileDialog::getOpenFileName(this, "Choose a file", get_current_dir_name(), "Pictures (*.bmp *.BMP *.png *.PNG)");
   }
   setDisabled(false);
   if (!newFile.isEmpty() && !newFile.isNull()) {
      setFile(newFile);
   }
}

void QFileChooser::setFile(const QString & fileName) {
   emit(fileChanged(fileName));
}
