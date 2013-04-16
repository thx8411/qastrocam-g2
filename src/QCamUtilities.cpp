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


#include "QCamUtilities.hpp"

#include <Qt/qpixmap.h>
#include <Qt/qwidget.h>
#include <Qt/qtranslator.h>
#include <Qt/qtextcodec.h>
#include <Qt/qapplication.h>

#include <libgen.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>

#include <iostream>

string QCamUtilities::basePath_="/usr/share/qastrocam-g2/";
bool QCamUtilities::useSDL_=false;
bool QCamUtilities::expertMode_=false;
QPalette* QCamUtilities::stdPalette=NULL;
QPalette* QCamUtilities::nightPalette=NULL;
bool QCamUtilities::nightVision=false;
struct widgetItem* QCamUtilities::widgetList=NULL;

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

void QCamUtilities::setQastrocamIcon(QWidget *widget) {
   static QPixmap * iconNb=QCamUtilities::getIcon("qastrocam-icon-nb.png");
   if(iconNb)
      widget->setIcon(*iconNb);
}

void QCamUtilities::setLocale(QApplication & app) {
	 static QTranslator translator( 0 );
	 translator.load( QString("qastrocam-") + QTextCodec::locale(), (basePathName()+"/locales/.").c_str());
	 app.installTranslator( &translator );
}

// night vision

void QCamUtilities::registerWidget(QWidget* w) {
   if(nightVision)
      w->setPalette(*nightPalette);
   else
      w->setPalette(*stdPalette);
   struct widgetItem* tmp;
   tmp=(struct widgetItem*)malloc(sizeof(widgetItem));
   tmp->item=w;
   tmp->next=widgetList;
   widgetList=tmp;
   //cerr << "register" << endl;
}

void QCamUtilities::removeWidget(QWidget* w) {
   struct widgetItem* tmp;
   tmp=widgetList;

   // messy stuff, just for tests, must be rewritten
   while(tmp!=NULL) {
      if(tmp->item==w)
         tmp->item==NULL;
      tmp=tmp->next;
   }

   //cerr << "remove" << endl;
}

void QCamUtilities::setStdMode() {
   struct widgetItem* tmp;
   tmp=widgetList;
   while(tmp!=NULL) {
      if(tmp->item!=NULL) {
         tmp->item->setPalette(*stdPalette);
         tmp=tmp->next;
      }
   }
   nightVision=false;

  //cerr << "std mode" << endl;
}

void QCamUtilities::setNightMode() {
   struct widgetItem* tmp;
   tmp=widgetList;
   while(tmp!=NULL) {
      if(tmp->item!=NULL) {
         tmp->item->setPalette(*nightPalette);
         tmp=tmp->next;
      }
   }
   nightVision=true;

   //cerr << "night mode" << endl;
}
