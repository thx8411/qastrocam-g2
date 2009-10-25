#include "QDirectoryChooser.moc"

#include <qfiledialog.h>
#include <qtooltip.h>

#include <libgen.h> // for basename
#include <stdlib.h>
#include <unistd.h> // for get_current_dir_name

#include <iostream>

#include "QCamUtilities.hpp"

using namespace std;

QDirectoryChooser::QDirectoryChooser(QWidget * parent):
   QPushButton(parent) {
   char * curDir=get_current_dir_name();
   connect(this,SIGNAL(pressed()),this,SLOT(selectDirectory()));
   setDirectory(curDir);
   free(curDir);
   setPixmap(*QCamUtilities::getIcon("choose_directory.png"));
}

QDirectoryChooser::~QDirectoryChooser() {
}

void QDirectoryChooser::selectDirectory() {
   setDisabled(true);
   QString newDir = QFileDialog::getExistingDirectory(
      currentDir_,
      this,
      "get existing directory",
      "Choose a directory",
      TRUE );
   setDisabled(false);
   if (!newDir.isEmpty() && !newDir.isNull()) {
      setDirectory(newDir);
   }
}

void QDirectoryChooser::setDirectory(const QString & dirName) {
   currentDir_=dirName;
   char * tmpStr=strdup(dirName.latin1());
   //setText(basename(tmpStr));
   free(tmpStr);
   QToolTip::add(this, currentDir_);
   emit(directoryChanged(dirName));
}
