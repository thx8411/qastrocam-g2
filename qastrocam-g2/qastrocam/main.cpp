/******************************************************************
Qastrocam
Copyright (C) 2003-2009   Franck Sicard
Qastrocam-g2
Copyright (C) 2009-2010   Blaise-Florentin Collin

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

#include "stdlib.h"

#include "QCamSlider.hpp"
#include "QCamV4L.hpp"
#include "QCamVesta.hpp"
#include "qastrocamVersion.hpp"
#include "QCamAdd.hpp"
#include "QCamMax.hpp"
#include "FrameMirror.hpp"
#include "QCamTrans.hpp"
#include "CamHistogram.hpp"
#include <qapplication.h>
#include <qtabwidget.h>
#include <qvbox.h>
#include <qpushbutton.h>
#include <qmessagebox.h>

#include "QCamUtilities.hpp"

#include "QCamFindShift_barycentre.hpp"
#include "QCamFindShift_hotSpot.hpp"
#include "QCamAutoGuidageSimple.hpp"
#include "QCamAutoAlign.hpp"
#include "QTelescopeAutostar.hpp"
#include "QTelescopeMCU.hpp"
#include "QTelescopeAPM.hpp"
#include "QTelescopeFifo.hpp"
#include "QTelescopeFile.hpp"
#include "QTelescopeMTS.cpp"
#include "PPort.hpp"
#include "QKingClient.hpp"
#include "SettingsBackup.hpp"
#include "QSetting.hpp"

// options strings
const string AccumOptionString("-a");
const string MirrorOptionString("-M");
const string AutoAlignOptionString("-c");
const string MaxOptionString("-m");
const string KingOption("-K");
const string VideoDeviceOptionString("-dv");
const string TelescopeTypeOption("-t");
const string TelescopeDeviceOptionString("-dt");
const string LongexposureDeviceOptionString("-dx");
const string LxLevelsInvertedOptionString("--lx-levels-inverted");
const string LxLevelsNormalOptionString("--lx-levels-normal");
const string TsLevelsInvertedOptionString("--ts-levels-inverted");
const string TsLevelsNormalOptionString("--ts-levels-normal");
const string LibDirOptionString("--libdir");
const string SDLon("--sdl");
const string ExpertMode("--expert");
const string LogMode("--log");
const string ForceGeneric("--force-generic");
const string ForceSettings("-sf");

// backup object, present everywhere
settingsBackup settings;

// qastrocam-g2 usage
void usage(const char * progName) {
   cerr << "usage: "<< progName << " <options>"<< endl;
   cerr << "\nValid options are:"<< endl << endl;
   cerr << "  "<<ForceSettings<<" to set the settings file name to use\n\n";
   cerr << "  "<<MirrorOptionString<<" to swap left/right top/bottom of the image\n";
   cerr << "  "<<AccumOptionString<<" to stack the images\n";
   cerr << "  "<<MaxOptionString<<" to simulate very long exposure on fixed mount\n";
   cerr << "     It keeps the max of each pixel\n";
   cerr << "  "<<AutoAlignOptionString<<" for options "<<AccumOptionString<<" & "<<MaxOptionString<<" align the frames before stacking\n";
   cerr << "  "<<KingOption<<" help to align the telescope with the king method.\n\n";
   cerr << "  "<<VideoDeviceOptionString << " <deviceName> to choose the V4L device name.\n" << "     default is /dev/video0.\n";
   cerr << "  "<<TelescopeTypeOption<<" <type> to select the telescope type\n" << "     type 'help' will give the list of avaible telescope type\n";
   cerr << "  "<<TelescopeDeviceOptionString << " <deviceName> to choose the telescope control device or file.\n" << "     default is /dev/ttyS1.\n";
   cerr << "  "<<LongexposureDeviceOptionString << " <deviceName> to choose de long exposure port (serial only).\n" << "     default is /dev/ttyS0 or /dev/parport0.\n\n";
   cerr << "  "<<LxLevelsInvertedOptionString<<" to invert polarity levels for serial and LED long exposure mods\n";
   cerr << "  "<<LxLevelsNormalOptionString<<" reset long exposure levels to non-inverted\n";
   cerr << "  "<<TsLevelsInvertedOptionString<<" to invert polarity levels for APM telescope\n";
   cerr << "  "<<TsLevelsNormalOptionString<<" reset APM levels to non-inverted\n\n";
   cerr << "  "<<LibDirOptionString<<" <directory> to set the library directory\n";
   cerr << "  "<<SDLon<<" use lib SDL to display frames (fast display).\n";
   cerr << "  "<<ExpertMode<<" enable some 'expert' options in the GUI\n";
   cerr << "  "<<LogMode<<" Logs qastrocam-g2 in a file for debug purpose\n";
   cerr << "  "<<ForceGeneric<<" <yes/no> to force usage of V4L generic module.\n";
   cerr << endl;
}

// tab widget
QTabWidget* getAllRemoteCTRL(QWidget* parent=0) {
   static QTabWidget* allRemote = new QTabWidget(parent,"allRemoteCTRL");
   return allRemote;
}

// add tab for cam object
void addRemoteCTRL(QCam* cam) {
   getAllRemoteCTRL()->addTab(cam->buildGUI(getAllRemoteCTRL()),cam->label());
}

// add tab for client object
void addRemoteCTRL(QCamClient* client) {
   getAllRemoteCTRL()->addTab(client->buildGUI(getAllRemoteCTRL()),client->label());
}

// add tab for setting widget
void addRemoteCTRL(QSetting* object) {
   getAllRemoteCTRL()->addTab(object->buildGUI(getAllRemoteCTRL()),object->label());
}

int main(int argc, char ** argv) {
   int i;
   // default options values
   bool accum=false,max=false,mirror=false;
   bool autoAlign=false;
   bool telescope=false;
   bool kingOption=false;
   bool V4Lforce=false;
   string videoDeviceName("/dev/video0");
   string telescopeType;
   string telescopeDeviceName("/dev/ttyS1");
   string libPath;
   string settingsFileName(".qastrocam-g2.conf");
   string logFileName("/dev/null");

   // looking for settings name option first
   for(i=1;i<argc;i++) {
      if (ForceSettings== argv[i]) {
         ++i;
         if (i==argc) {
            usage(argv[0]);
            exit(1);
         }
         settingsFileName=argv[i];
      }
   }

   // creating setting object
   settings.setName(settingsFileName);

   // reading settings
   settings.deSerialize();

   // set telescope device, using settings
   if(settings.haveKey("TELESCOPE_DEVICE")) telescopeDeviceName=settings.getKey("TELESCOPE_DEVICE");

   // decode all options
   for (i=1;i <argc;++i) {
      if (MirrorOptionString == argv[i]) {
         mirror=true;
      }  else if (AccumOptionString == argv[i]) {
         accum=true;
      } else if (MaxOptionString == argv[i]) {
         max=true;
      } else if (AutoAlignOptionString == argv[i]) {
         autoAlign=true;
      } else if (SDLon == argv[i]) {
         QCamUtilities::useSDL(true);
      } else if (ExpertMode == argv[i]) {
         QCamUtilities::expertMode(true);
      } else if (LogMode == argv[i]) {
         char buff[30];
         time_t timet;
         time(&timet);
         struct tm * t=gmtime(&timet);
         snprintf(buff,30,"%04d.%02d.%02d-%02dh%02dm%02ds",
            t->tm_year+1900,t->tm_mon+1,t->tm_mday,
            t->tm_hour,t->tm_min,t->tm_sec);
         logFileName="qastrocam-g2-"+string(buff)+".log";
      } else if (ForceGeneric == argv[i]) {
         i++;
         if (i==argc) {
            usage(argv[0]);
            exit(1);
         }
         if(strcasecmp("yes",argv[i])==0)
            settings.setKey("FORCE_V4LGENERIC","yes");
         else
            settings.setKey("FORCE_V4LGENERIC","no");
      } else if (VideoDeviceOptionString == argv[i]) {
         ++i;
         if (i==argc) {
            usage(argv[0]);
            exit(1);
         }
	 videoDeviceName=argv[i];
      } else if ( TelescopeTypeOption == argv[i]) {
         ++i;
         if (i==argc) {
            usage(argv[0]);
            exit(1);
         }
	 telescopeType=argv[i];
         telescope=true;
      } else if ( TelescopeDeviceOptionString == argv[i]) {
         ++i;
          if (i==argc) {
            usage(argv[0]);
            exit(1);
         }
	 telescopeDeviceName=argv[i];
         settings.setKey("TELESCOPE_DEVICE",argv[i]);
      } else if ( LongexposureDeviceOptionString == argv[i]) {
	 ++i;
	  if(i==argc) {
	    usage(argv[0]);
            exit(1);
	 }
         settings.setKey("LX_DEVICE",argv[i]);
      } else if ( LxLevelsInvertedOptionString == argv[i]) {
         settings.setKey("LX_LEVELS_INVERTED","yes");
      } else if ( LxLevelsNormalOptionString == argv[i]) {
         settings.setKey("LX_LEVELS_INVERTED","no");
      } else if ( TsLevelsInvertedOptionString == argv[i]) {
         settings.setKey("TS_LEVELS_INVERTED","yes");
      } else if ( TsLevelsNormalOptionString == argv[i]) {
         settings.setKey("TS_LEVELS_INVERTED","no");
      } else if ( LibDirOptionString == argv[i]) {
         ++i;
         if (i==argc) {
            usage(argv[0]);
            exit(1);
         }
         libPath=argv[i];
      } else if ( KingOption == argv[i]) {
         kingOption=true;
      } else if (ForceSettings== argv[i]) {
         ++i;
         if (i==argc) {
            usage(argv[0]);
            exit(1);
         }
         // nothing to be done
         // allready scanned
      } else {
         cerr << "Invalid option '"<<argv[i]<<"'"<<endl;
         usage(argv[0]);
         exit(1);
      }
   }

   // displays telescope liste
   if (telescopeType == "help") {
      cerr << "supported scopes:\n"
           << "* apm\n"
           << "* autostar\n"
           << "* fifo\n"
           << "* mcu\n"
           << "* mts\n"
	   << "* file\n";
      exit(0);
   }

   // cout redirection
   FILE* logFile=freopen(logFileName.c_str(),"w",stdout);

   // for log trace
   cout << qastrocamName << " " << qastroCamVersion
        << " (build "<<qastrocamBuild<<")"<<endl;
   cout << "* based on " << QCamUtilities::getVersionId() <<endl;
   cout << "* " << qastrocamWeb << endl;
   cout << "* " << qastrocamMail << endl;

   // setting path
   if (libPath.empty()) {
      QCamUtilities::computePathName(argv[0]);
   } else {
      QCamUtilities::basePathName(libPath);
   }
   cout << "* lib directory "<<QCamUtilities::basePathName()<<"\n";
   cout << endl;

   // use SDL messages
   if (QCamUtilities::useSDL())
      cout << "SDL display enabled. (If only a black windows is displayed, try without " << SDLon << ")" << endl;

   // QT app settings
   QApplication app(argc,argv);
   QCamUtilities::setLocale(app);

   // main window setting
   QPixmap* tmpIcon;
   QVBox mainWindow;
   QPushButton quit(&mainWindow,"Quit");
   QObject::connect( &quit, SIGNAL(released()), &app, SLOT(quit()) );
   tmpIcon=QCamUtilities::getIcon("exit.png");
   quit.setPixmap(*tmpIcon);
   delete tmpIcon;
   app.setMainWidget(&mainWindow);
   getAllRemoteCTRL(&mainWindow);

   // creating telescope object
   QTelescope * theTelescope=NULL;
   if (telescopeType.length() != 0) {
      if (telescopeType=="autostar") {
         theTelescope = new QTelescopeAutostar(telescopeDeviceName.c_str());
      } else if (telescopeType=="mcu") {
         theTelescope = new QTelescopeMCU(telescopeDeviceName.c_str());
      } else if (telescopeType=="apm") {
         theTelescope = new QTelescopeAPM(telescopeDeviceName.c_str());
      } else if (telescopeType=="fifo") {
         theTelescope = new QTelescopeFifo(telescopeDeviceName.c_str());
      } else if (telescopeType=="mts") {
         theTelescope = new QTelescopeMTS(telescopeDeviceName.c_str());
      } else if (telescopeType=="file") {
	 theTelescope = new QTelescopeFile(telescopeDeviceName.c_str());
      } else {
         cerr << "unsupported telescope type "
              <<"'"<<telescopeType<<"'"<<endl;
         usage(argv[0]);
         exit(1);
      }
      theTelescope->buildGUI();
   }

   // is generic V4L forced in settings ?
   if(settings.haveKey("FORCE_V4LGENERIC")) {
      if(strcasecmp(settings.getKey("FORCE_V4LGENERIC"),"yes")==0)
         V4Lforce=true;
   }

   // capture module creation
   QCam* cam =NULL;
   cam = QCamV4L::openBestDevice(videoDeviceName.c_str(),V4Lforce);
   if (cam == NULL) {
      QMessageBox::information(0,"Qastrocam-g2","No camera detected\nSettings panel only");
      cout << "No camera detected" <<endl;
   }

   QKingClient * kingClient=NULL;
   QCamFindShift * findShift=NULL;
   QCamAutoGuidage * tracker=NULL;
   QCamAutoAlign * autoAlignCam=NULL;
   QCamTrans  * camMirror=NULL;
   FrameMirror * mirrorAlgo=NULL;
   QCam* camAdd=NULL;
   QCam* camMax=NULL;

   if(cam!=NULL) {
      addRemoteCTRL(cam);
      cam->setCaptureFile("raw");
      QCam * camSrc=cam;

      // King client object creation
      if (kingOption) {
         cout << "King aligment enabled\n";
         kingClient=new QKingClient();
         sleep(1);
         kingClient->connectCam(*camSrc);
         //addRemoteCTRL(kingClient);
         kingClient->buildGUI(NULL)->show();
      }

      // alignement object
      if (theTelescope || autoAlign) {
         //QCamFindShift * baryShift=new QCamFindShift_barycentre();
         findShift=new QCamFindShift_hotSpot(theTelescope);
         findShift->connectCam(*camSrc);
      }
      if (theTelescope) {
         tracker = new QCamAutoGuidageSimple();
         tracker->setCam(camSrc);
         tracker->setTracker(findShift);
         tracker->setScope(theTelescope);
         //GUI build later
         //tracker->buildGUI();
      }
      // autoaligne creation
      if (autoAlign) {
         assert(findShift);
         autoAlignCam=new QCamAutoAlign();
         autoAlignCam->setTracker(findShift);
         addRemoteCTRL(autoAlignCam);
         camSrc=autoAlignCam;
      }

      if (tracker) {
         if (autoAlign) {
            tracker->buildGUI(camSrc->gui());
         } else {
            tracker->buildGUI();
         }
      }

      // mirror module
      if (mirror) {
         camMirror = new QCamTrans();
         mirrorAlgo = new FrameMirror();
         camMirror->connectCam(*camSrc);
         camMirror->connectAlgo(*mirrorAlgo);
         addRemoteCTRL(camMirror);
         camMirror->setCaptureFile("mirror");
         camSrc=camMirror;
      }

      // accumulation module
      if (accum) {
         camAdd = new QCamAdd(camSrc);
         addRemoteCTRL(camAdd);
         camAdd->setCaptureFile("add");
         camSrc=camAdd;
      }
      if (max) {
         //creation du module d'acumulation
         camMax = new QCamMax(camSrc);
         addRemoteCTRL(camMax);
         camMax->setCaptureFile("max");
         camSrc=camMax;
      }
   }

   // settings tab
   QSetting* settingsTab;
   settingsTab= new QSetting();
   addRemoteCTRL(settingsTab);

   // main window display
   QCamUtilities::setQastrocamIcon(&mainWindow);
   mainWindow.show();
   mainWindow.adjustSize();
   getAllRemoteCTRL()->show();
   getAllRemoteCTRL()->adjustSize();

   // QT event loop
   app.exec();

   // release all
   delete settingsTab;
   delete theTelescope;
   delete kingClient;
   delete findShift;
   delete tracker;
   delete autoAlignCam;
   delete camMirror;
   delete mirrorAlgo;
   delete camAdd;
   delete camMax;
   delete cam;
   return(0);
}
