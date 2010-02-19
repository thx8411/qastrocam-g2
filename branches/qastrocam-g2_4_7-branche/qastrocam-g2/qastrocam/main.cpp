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

#include "../config.h"

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
const string LxLevelsInvertedOptionString("--lx-levels");
const string TsLevelsInvertedOptionString("--ts-levels");
const string LibDirOptionString("--libdir");
const string SDLon("--sdl");
const string ExpertMode("--expert");
const string LogMode("--log");
//const string ForceGeneric("--force-generic");
const string ForceSettings("-sf");

// backup object, present everywhere
settingsBackup settings;

// qastrocam-g2 usage
void usage(const char * progName) {
   cerr << "usage: "<< progName << " <options>"<< endl;
   cerr << "\nValid options are:"<< endl << endl;
   cerr << "  "<<ForceSettings<<" to set the settings file name to use\n\n";
   cerr << "  "<<MirrorOptionString<<" <yes/no> to swap left/right top/bottom of the image\n";
   cerr << "  "<<AccumOptionString<<" <yes/no> to stack the images\n";
   cerr << "  "<<MaxOptionString<<" <yes/no> to simulate very long exposure on fixed mount\n";
   cerr << "     It keeps the max of each pixel\n";
   cerr << "  "<<AutoAlignOptionString<<" <yes/no> for options "<<AccumOptionString<<" & "<<MaxOptionString<<" align the frames before stacking\n";
   cerr << "  "<<KingOption<<" <yes/no> help to align the telescope with the king method.\n\n";
   cerr << "  "<<VideoDeviceOptionString << " <deviceName> to choose the V4L device name.\n" << "     default is /dev/video0.\n";
   cerr << "  "<<TelescopeTypeOption<<" <type> to select the telescope type\n" << "     type 'help' will give the list of avaible telescope type\n";
   cerr << "  "<<TelescopeDeviceOptionString << " <deviceName> to choose the telescope control device or file.\n" << "     default is /dev/ttyS1.\n";
   cerr << "  "<<LongexposureDeviceOptionString << " <deviceName> to choose de long exposure port (serial only).\n" << "     default is /dev/ttyS0 or /dev/parport0.\n\n";
   cerr << "  "<<LxLevelsInvertedOptionString<<" <yes/no> to invert polarity levels for serial and LED long exposure mods\n";
   cerr << "  "<<TsLevelsInvertedOptionString<<" <yes/no> to invert polarity levels for APM telescope\n";
   cerr << "  "<<LibDirOptionString<<" <directory> to set the library directory\n";
   cerr << "  "<<SDLon<<" <yes/no> use lib SDL to display frames (fast display).\n";
   cerr << "  "<<ExpertMode<<" <yes/no> enable some 'expert' options in the GUI\n";
   cerr << "  "<<LogMode<<" <yes/no> Logs qastrocam-g2 in a file for debug purpose\n";
//   cerr << "  "<<ForceGeneric<<" <yes/no> to force usage of V4L generic module.\n";
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
   bool kingOption=false;
//   bool V4Lforce=false;
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

   // decode all options
   for (i=1;i <argc;++i) {
      // settings file
      if (ForceSettings== argv[i]) {
         ++i;
         if (i==argc) {
            usage(argv[0]);
            exit(1);
         }
         // nothing to be done
         // allready scanned

      // cam clients options
      } else if (MirrorOptionString == argv[i]) {
         i++;
         if (i==argc) {
            usage(argv[0]);
            exit(1);
         }
         if(strcasecmp("yes",argv[i])==0)
            settings.setKey("MIRROR_MODULE","yes");
         else
            settings.setKey("MIRROR_MODULE","no");
      } else if (AccumOptionString == argv[i]) {
         i++;
         if (i==argc) {
            usage(argv[0]);
            exit(1);
         }
         if(strcasecmp("yes",argv[i])==0)
            settings.setKey("ADD_MODULE","yes");
         else
            settings.setKey("ADD_MODULE","no");
      } else if (MaxOptionString == argv[i]) {
         i++;
         if (i==argc) {
            usage(argv[0]);
            exit(1);
         }
         if(strcasecmp("yes",argv[i])==0)
            settings.setKey("MAX_MODULE","yes");
         else
            settings.setKey("MAX_MODULE","no");
      } else if (AutoAlignOptionString == argv[i]) {
         i++;
         if (i==argc) {
            usage(argv[0]);
            exit(1);
         }
         if(strcasecmp("yes",argv[i])==0)
            settings.setKey("ALIGN_MODULE","yes");
         else
            settings.setKey("ALIGN_MODULE","no");
      } else if ( KingOption == argv[i]) {
         i++;
         if (i==argc) {
            usage(argv[0]);
            exit(1);
         }
         if(strcasecmp("yes",argv[i])==0)
            settings.setKey("KING_MODULE","yes");
         else
            settings.setKey("KING_MODULE","no");
//      } else if (ForceGeneric == argv[i]) {
//         i++;
//         if (i==argc) {
//            usage(argv[0]);
//            exit(1);
//         }
//         if(strcasecmp("yes",argv[i])==0)
//            settings.setKey("FORCE_V4LGENERIC","yes");
//         else
//            settings.setKey("FORCE_V4LGENERIC","no");
      } else if ( TelescopeDeviceOptionString == argv[i]) {
         ++i;
          if (i==argc) {
            usage(argv[0]);
            exit(1);
         }
         settings.setKey("TELESCOPE_DEVICE",argv[i]);
      } else if ( LongexposureDeviceOptionString == argv[i]) {
         ++i;
          if(i==argc) {
            usage(argv[0]);
            exit(1);
         }
         settings.setKey("LX_DEVICE",argv[i]);
      } else if ( LxLevelsInvertedOptionString == argv[i]) {
         ++i;
          if(i==argc) {
            usage(argv[0]);
            exit(1);
         }
         settings.setKey("LX_LEVELS_INVERTED",argv[i]);
      } else if ( TsLevelsInvertedOptionString == argv[i]) {
         ++i;
          if(i==argc) {
            usage(argv[0]);
            exit(1);
         }
         settings.setKey("TS_LEVELS_INVERTED",argv[i]);
      } else if (VideoDeviceOptionString == argv[i]) {
         ++i;
         if (i==argc) {
            usage(argv[0]);
            exit(1);
         }
         settings.setKey("VIDEO_DEVICE",argv[i]);
      } else if (SDLon == argv[i]) {
         ++i;
          if(i==argc) {
            usage(argv[0]);
            exit(1);
         }
         settings.setKey("SDL",argv[i]);
      } else if (ExpertMode == argv[i]) {
         ++i;
          if(i==argc) {
            usage(argv[0]);
            exit(1);
         }
         settings.setKey("EXPERT",argv[i]);
       } else if (VideoDeviceOptionString == argv[i]) {
         ++i;
         if (i==argc) {
            usage(argv[0]);
            exit(1);
         }
         settings.setKey("VIDEO_DEVICE",argv[i]);
      } else if (LogMode == argv[i]) {
         ++i;
          if(i==argc) {
            usage(argv[0]);
            exit(1);
         }
         settings.setKey("LOG",argv[i]);
      } else if ( LibDirOptionString == argv[i]) {
         ++i;
         if (i==argc) {
            usage(argv[0]);
            exit(1);
         }
         settings.setKey("LIB_PATH",argv[i]);
      } else if ( TelescopeTypeOption == argv[i]) {
         ++i;
         if (i==argc) {
            usage(argv[0]);
            exit(1);
         }
	 settings.setKey("TELESCOPE",argv[i]);
      } else {
         cerr << "Invalid option '"<<argv[i]<<"'"<<endl;
         usage(argv[0]);
         exit(1);
      }
   }

   // set devices/options, using settings
   if(settings.haveKey("VIDEO_DEVICE")) videoDeviceName=settings.getKey("VIDEO_DEVICE");
   if(settings.haveKey("TELESCOPE_DEVICE")) telescopeDeviceName=settings.getKey("TELESCOPE_DEVICE");
   if(settings.haveKey("SDL")&&string(settings.getKey("SDL"))=="yes") QCamUtilities::useSDL(true);
   if(settings.haveKey("EXPERT")&&string(settings.getKey("EXPERT"))=="yes") QCamUtilities::expertMode(true);
   if(settings.haveKey("LOG")&&string(settings.getKey("LOG"))=="yes") {
      char buff[30];
         time_t timet;
         time(&timet);
         struct tm * t=gmtime(&timet);
         snprintf(buff,30,"%04d.%02d.%02d-%02dh%02dm%02ds",
            t->tm_year+1900,t->tm_mon+1,t->tm_mday,
            t->tm_hour,t->tm_min,t->tm_sec);
         logFileName="qastrocam-g2-"+string(buff)+".log";
   }
   if(settings.haveKey("LIB_PATH")) libPath=settings.getKey("LIB_PATH");
   if(settings.haveKey("TELESCOPE")) telescopeType=settings.getKey("TELESCOPE");
   if(settings.haveKey("ADD_MODULE")&&string(settings.getKey("ADD_MODULE"))=="yes") accum=true;
   if(settings.haveKey("MAX_MODULE")&&string(settings.getKey("MAX_MODULE"))=="yes") max=true;
   if(settings.haveKey("MIRROR_MODULE")&&string(settings.getKey("MIRROR_MODULE"))=="yes") mirror=true;
   if(settings.haveKey("ALIGN_MODULE")&&string(settings.getKey("ALIGN_MODULE"))=="yes") autoAlign=true;
   if(settings.haveKey("KING_MODULE")&&string(settings.getKey("KING_MODULE"))=="yes") kingOption=true;

   // displays telescope liste
   if (telescopeType == "help") {
      cerr << "supported scopes:\n"
           << "* none\n"
           << "* apm\n"
           << "* autostar\n"
           << "* fifo\n"
           << "* mcu\n"
           << "* mts\n"
	   << "* file\n";
      exit(0);
   }

   // cout redirection
#ifndef _DEBUG_
   FILE* logFile=freopen(logFileName.c_str(),"w",stdout);
#endif

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
   QString caption;
   QPixmap* tmpIcon;
   QVBox mainWindow;
   caption=qastrocamName;
   caption+=" ";
   caption+=qastroCamVersion;
   mainWindow.setCaption(caption);
   QPushButton quit(&mainWindow,"Quit");
   QObject::connect( &quit, SIGNAL(released()), &app, SLOT(quit()) );
   tmpIcon=QCamUtilities::getIcon("exit.png");
   quit.setPixmap(*tmpIcon);
   delete tmpIcon;
   app.setMainWidget(&mainWindow);
   getAllRemoteCTRL(&mainWindow);

   // creating telescope object
   if(telescopeType=="none") telescopeType="";
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
//   if(settings.haveKey("FORCE_V4LGENERIC")) {
//      if(strcasecmp(settings.getKey("FORCE_V4LGENERIC"),"yes")==0)
//         V4Lforce=true;
//   }

   // capture module creation
   QCam* cam =NULL;
   cam = QCamV4L::openBestDevice(videoDeviceName.c_str());
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

#ifndef _DEBUG_
   fclose(logFile);
#endif
   return(0);
}
