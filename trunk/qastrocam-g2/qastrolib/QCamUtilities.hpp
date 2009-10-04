#ifndef _QCamUtilities_hpp_
#define _QCamUtilities_hpp_

#include <string>

using namespace std;

class QPixmap;
class QLabel;
class QWidget;
class QApplication;

class QCamUtilities {
public:
   static QPixmap * getIcon(const char * pixmapBaseName);
   static void computePathName(const char * pathToBinary);
   static const string & basePathName() { return basePath_;}
   static const string getVersionId();
   static void setQastrocamIcon(QWidget *);
   static void basePathName(const string & path) {basePath_=path;}
   static void useSDL(bool val) { useSDL_=val;}
   static bool useSDL() {return useSDL_;}
   static void setLocale(QApplication & app);
   static void expertMode(bool val) { expertMode_=val;}
   static bool expertMode() { return expertMode_; }
private:
   static string basePath_;
   static bool useSDL_;
   // in expert mode, more controls are displayed in the GUI
   static bool expertMode_;
};

#endif
