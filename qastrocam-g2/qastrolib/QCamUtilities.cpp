#include "QCamUtilities.hpp"
#include "qastrolibVersion.hpp"

#include <qpixmap.h>
#include <qwidget.h>
#include <qtranslator.h>
#include <qtextcodec.h>
#include <qapplication.h>

#include <libgen.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>

#include <iostream>

string QCamUtilities::basePath_="/usr/share/qastrocam-g2/";

bool QCamUtilities::useSDL_=false;
bool QCamUtilities::expertMode_=false;

void QCamUtilities::computePathName(const char * path) {
   char* tmp;
   DIR * tmpDir=opendir(basePath_.c_str());
   if (tmpDir != NULL) {
       // curren path is good
       closedir(tmpDir);
       return;
   }
   //cout <<"pass 1: "<<path<<"\n";
   char * workingCopy=strdup(path);
   basePath_=dirname(workingCopy);
   //cout <<"pass 2: "<<basePath_<<"\n";
   if (basePath_[0]!='/') {
      char cwd[512];
      tmp=getcwd(cwd,512);
      basePath_=string(cwd)+(basePath_!="."?"/"+basePath_:"");
   }
   //cout <<"pass 2 bis: "<<basePath_<<"\n";
   free(workingCopy);
   workingCopy=strdup(basePath_.c_str());
   basePath_=dirname(workingCopy);
   //cout <<"pass 3: "<<basePath_<<"\n";
}

QPixmap * QCamUtilities::getIcon(const char * pixmapBaseName) {
   string fullPath=basePathName()
                   +"/icons/"
                   +pixmapBaseName;
   QPixmap * pixmap=new QPixmap(fullPath.c_str());
   if (pixmap->isNull()) {
      cerr << "QCamUtilities::getIcon() :"
           << fullPath
           << " is not a valid file for pixmap"<<endl;
      delete pixmap;
      pixmap=NULL;
   }
   return pixmap;
}

const string QCamUtilities::getVersionId() {
   static string versionId= string(qastrolibName)
                     + " " +  qastrolibVersion;
   return versionId;
}

void QCamUtilities::setQastrocamIcon(QWidget *widget) {
   static QPixmap * iconNb=QCamUtilities::getIcon("qastrocam-icon-nb.png");
   widget->setIcon(*iconNb);
}

void QCamUtilities::setLocale(QApplication & app) {
	 static QTranslator translator( 0 );
	 translator.load( QString("qastrocam-") + QTextCodec::locale(), (basePathName()+"/locales/.").c_str());
	 app.installTranslator( &translator );
}
