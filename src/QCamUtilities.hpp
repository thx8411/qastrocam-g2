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


#ifndef _QCamUtilities_hpp_
#define _QCamUtilities_hpp_

#include <string>

#include <Qt/qpalette.h>
//Added by qt3to4:
#include <Qt/qpixmap.h>
#include <Qt/qlabel.h>

using namespace std;

class QPixmap;
class QLabel;
class QWidget;
class QApplication;

struct widgetItem {
   QWidget* item;
   widgetItem* next;
};

class QCamUtilities {
public:
   static QPixmap * getIcon(const char * pixmapBaseName);
   static void computePathName(const char * pathToBinary);
   static const string & basePathName() { return basePath_;}
   static void setQastrocamIcon(QWidget *);
   static void basePathName(const string & path) {basePath_=path;}
   static void useSDL(bool val) { useSDL_=val;}
   static bool useSDL() {return useSDL_;}
   static void setLocale(QApplication & app);
   static void expertMode(bool val) { expertMode_=val;}
   static bool expertMode() { return expertMode_; }
   static void registerWidget(QWidget* w);
   static void removeWidget(QWidget* w);
   static void setStdMode();
   static void setNightMode();
   static bool nightVision;
   static QPalette* stdPalette;
   static QPalette* nightPalette;
private:
   static struct widgetItem* widgetList;
   static string basePath_;
   static bool useSDL_;
   // in expert mode, more controls are displayed in the GUI
   static bool expertMode_;
};

#endif
