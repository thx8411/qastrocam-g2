/******************************************************************
Qastrocam
Copyright (C) 2003-2009   Franck Sicard
Qastrocam-g2
Copyright (C) 2009-2014   Blaise-Florentin Collin

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
// for kernel detection
#include <sys/utsname.h>

#include <Qt/qpixmap.h>
#include <Qt/qapplication.h>
#include <Qt/qtabwidget.h>
#include <Qt/qpushbutton.h>
#include <Qt/qmessagebox.h>
#include <Qt/qpalette.h>

#if HAVE_SDL_H
#include <SDL.h>
#endif

#include "QCamWindow.hpp"
#include "QCamVBox.hpp"
#include "QCamSlider.hpp"
#include "QCamSimulator.hpp"
#include "QCamQHY5.hpp"
#include "QCamQHY6.hpp"
#include "QCamV4L2.hpp"
#include "qastrocamVersion.hpp"
#include "QCamAdd.hpp"
#include "QCamMax.hpp"
#include "FrameMirror.hpp"
#include "FrameBias.hpp"
#include "FrameDark.hpp"
#include "FrameFlat.hpp"
#include "FrameBayer.hpp"
#include "FrameId.hpp"
#include "QCamTrans.hpp"
#include "CamHistogram.hpp"
#include "QCamUtilities.hpp"
#include "QCamFindShift_barycentre.hpp"
#include "QCamFindShift_hotSpot.hpp"
#include "QCamAutoGuidageSimple.hpp"
#include "QCamAutoAlign.hpp"
#include "QTelescopeAutostar.hpp"
#include "QTelescopeNexstar.hpp"
#include "QTelescopeMCU.hpp"
#include "QTelescopeAPM.hpp"
#include "QTelescopeFifo.hpp"
#include "QTelescopeFile.hpp"
#include "QTelescopeMTS.hpp"
#include "QTelescopeQHY5.hpp"
#include "QTelescopeQHY6.hpp"
#include "QTelescopeLX200.hpp"
#include "QTelescopeSimulator.hpp"
#include "PPort.hpp"
#include "QHY5cam.hpp"
#include "QHY6cam.hpp"
#include "QKingClient.hpp"
#include "SettingsBackup.hpp"
#include "QSetting.hpp"
#include "QCamStack.hpp"
#include "QCamFocus.hpp"


// options strings
const string AccumOptionString("-a");
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
const string ForceSettings("--sf");
const string NightVision("--nv");

// backup object, present everywhere
settingsBackup settings;

// app pointer, present everywhere
QApplication* appPointer;

// qastrocam-g2 usage
void usage(const char * progName) {
   cerr << "usage: "<< progName << " <options>"<< endl;
   cerr << "\nValid options are:"<< endl << endl;
   cerr << "  "<<ForceSettings<<" to set the settings file name to use\n\n";
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
   cerr << "  "<<NightVision<<" <yes/no> to enable or disable night vision mode\n";
   cerr << endl;
}

// tab widget
QTabWidget* getAllRemoteCTRL(QWidget* parent=0) {
   static QTabWidget* allRemote = new QTabWidget(parent);
   return allRemote;
}

// add tab for cam object
void addRemoteCTRL(QCam* cam) {
   getAllRemoteCTRL()->addTab(cam->buildGUI(getAllRemoteCTRL()),cam->label());
}

// add tab for cam stack object
void addRemoteCTRL(QCamStack* stack) {
   getAllRemoteCTRL()->addTab(stack->buildGUI(getAllRemoteCTRL()),stack->label());
}

// add tab for client object
void addRemoteCTRL(QCamClient* client) {
   getAllRemoteCTRL()->addTab(client->buildGUI(getAllRemoteCTRL()),client->label());
}

// add tab for setting widget
void addRemoteCTRL(QSetting* object) {
   getAllRemoteCTRL()->addTab(object->buildGUI(getAllRemoteCTRL()),object->label());
}

// add tab for focus widget
void addRemoteCTRL(QCamFocus* object) {
   getAllRemoteCTRL()->addTab(object->buildGUI(getAllRemoteCTRL()),object->label());
}


int main(int argc, char ** argv) {
   int i;
   // default options values
   bool accum=false,max=false;
   bool autoAlign=false;
   bool kingOption=false;
   string cameraName("simulator");
   string videoDeviceName("/dev/video0");
   string telescopeType;
   string telescopeDeviceName("/dev/ttyS1");
   string libPath;
   string settingsFileName(".qastrocam-g2.conf");
   string logFileName("/dev/null");

#if HAVE_SDL_H
   // SDL init
   //SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO);
#endif

   // getting the kernel version
   utsname kernel_info;
   int kernel_version,kernel_revision,kernel_patch;
   uname(&kernel_info);
   sscanf(kernel_info.release,"%i.%i.%i",&kernel_version,&kernel_revision,&kernel_patch);

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
   if(!settings.deSerialize()) {
      // no file -> new file, add the version key
      settings.setKey("CONF_VERSION",_CONF_VERSION_);
   }

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
         settings.setKey("CAMERA_DEVICE",argv[i]);
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
         settings.setKey("CAMERA_DEVICE",argv[i]);
      } else if (LogMode == argv[i]) {
         ++i;
         if(i==argc) {
            usage(argv[0]);
            exit(1);
         }
         settings.setKey("LOG",argv[i]);
      } else if (NightVision == argv[i]) {
         ++i;
         if(i==argc) {
            usage(argv[0]);
            exit(1);
         }
         settings.setKey("NIGHT_VISION",argv[i]);
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
   if(settings.haveKey("CAMERA")) cameraName=settings.getKey("CAMERA");
   if(settings.haveKey("CAMERA_DEVICE")) videoDeviceName=settings.getKey("CAMERA_DEVICE");
   if(settings.haveKey("TELESCOPE_DEVICE")) telescopeDeviceName=settings.getKey("TELESCOPE_DEVICE");
#if HAVE_SDL_H
   if(settings.haveKey("SDL")&&string(settings.getKey("SDL"))=="yes") QCamUtilities::useSDL(true);
#endif
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
   if(settings.haveKey("NIGHT_VISION")&&string(settings.getKey("NIGHT_VISION"))=="yes") QCamUtilities::setNightMode();
   if(settings.haveKey("LIB_PATH")) libPath=settings.getKey("LIB_PATH");
   if(settings.haveKey("TELESCOPE")) telescopeType=settings.getKey("TELESCOPE");
   if(settings.haveKey("ADD_MODULE")&&string(settings.getKey("ADD_MODULE"))=="yes") accum=true;
   if(settings.haveKey("MAX_MODULE")&&string(settings.getKey("MAX_MODULE"))=="yes") max=true;
   if(settings.haveKey("ALIGN_MODULE")&&string(settings.getKey("ALIGN_MODULE"))=="yes") autoAlign=true;
   if(settings.haveKey("KING_MODULE")&&string(settings.getKey("KING_MODULE"))=="yes") kingOption=true;

   // displays telescope liste
   if (telescopeType == "help") {
      cerr << "supported scopes:\n"
           << "* none\n"
           << "* qhy5\n"
           << "* qhy6\n"
           << "* apm\n"
           << "* autostar\n"
           << "* lx200\n"
           << "* nexstar\n"
           << "* fifo\n"
           << "* mcu\n"
           << "* mts\n"
           << "* simulator\n"
	   << "* file\n";
      exit(0);
   }

   // cout redirection
#ifndef _DEBUG_
   FILE* logFile=freopen(logFileName.c_str(),"w",stdout);
#endif

   // for log trace
   cout << qastrocamName << " " << qastroCamVersion
        << " (build "<< qastrocamBuild << ")" <<endl;
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
   QApplication::setStyle("Plastique");
   appPointer=&app;

   // kernel checking
   if(kernel_version==3 && KERNEL_2) {
      QMessageBox::information(0,"Qastrocam-g2","This binary can't run on a kernel 3.x.x, leaving...");
      app.quit();
      exit(1);
   }
   if(kernel_version==2 && !KERNEL_2) {
      QMessageBox::information(0,"Qastrocam-g2","This binary can't run on a kernel 2.x.x, leaving...");
      app.quit();
      exit(1);
   }

   // main window setting
   QString caption;
   QIcon* tmpIcon;
   QCamWindow mainWindow;
   QCamVBox mainWidget(&mainWindow);
   caption=qastrocamName;
   caption+=" ";
   caption+=qastroCamVersion;
   mainWidget.setWindowTitle(caption);
   QPushButton quit(&mainWidget);
   // quit button connected to app quit
   QObject::connect(&quit, SIGNAL(released()), &app, SLOT(quit()) );
   // if the main window is closed, app quit
   QObject::connect(&mainWindow, SIGNAL(windowClosed()), &app, SLOT(quit()));
   tmpIcon=QCamUtilities::getIcon("exit.png");
   if(tmpIcon) {
      quit.setIcon(*tmpIcon);
      delete tmpIcon;
   } else
      quit.setText("Quit");
   getAllRemoteCTRL(&mainWidget);

   // std palette
   QPalette tmpPalette=mainWidget.palette();
   // setting tooltips text to black
   tmpPalette.setColor(QPalette::ToolTipText, Qt::black);
   app.setPalette(tmpPalette);
   // store the std palette
   QCamUtilities::stdPalette=&tmpPalette;

   // night palette
   QPalette* nightPalette=new QPalette();
   nightPalette->setColor(QPalette::Active,QPalette::Background,Qt::darkRed);
   nightPalette->setColor(QPalette::Active,QPalette::Base,QColor(176,0,0));
   nightPalette->setColor(QPalette::Active,QPalette::Button,QColor(160,0,0));
   nightPalette->setColor(QPalette::Active,QPalette::HighlightedText,Qt::red);

   nightPalette->setColor(QPalette::Inactive,QPalette::Background,Qt::darkRed);
   nightPalette->setColor(QPalette::Inactive,QPalette::Base,QColor(176,0,0));
   nightPalette->setColor(QPalette::Inactive,QPalette::Button,QColor(160,0,0));
   nightPalette->setColor(QPalette::Inactive,QPalette::HighlightedText,Qt::red);

   nightPalette->setColor(QPalette::Disabled,QPalette::Background,Qt::darkRed);
   nightPalette->setColor(QPalette::Disabled,QPalette::Base,QColor(176,0,0));
   nightPalette->setColor(QPalette::Disabled,QPalette::Button,QColor(160,0,0));
   nightPalette->setColor(QPalette::Disabled,QPalette::HighlightedText,Qt::red);
   nightPalette->setColor(QPalette::Disabled,QPalette::Text,QColor(160,0,0));
   QCamUtilities::nightPalette=nightPalette;

   QCamUtilities::registerWidget(&mainWidget);

   // capture module creation
   QCam* cam = NULL;
   // test QHY5
#if HAVE_USB_H && HAVE_PTHREADS_H
   if(cameraName=="qhy5") {
      if(QHY5cam::plugged())
         cam = new QCamQHY5();
      else {
         QMessageBox::information(0,"Qastrocam-g2","QHY5 camera not detected\nSettings panel only");
         cout << "QHY5 camera not detected" << endl;
      }
   } else if(cameraName=="qhy6") {
      if(QHY6cam::plugged())
         cam = new QCamQHY6();
      else {
         QMessageBox::information(0,"Qastrocam-g2","QHY6 camera not detected\nSettings panel only");
         cout << "QHY6 camera not detected" << endl;
      }
   } else
#endif /* HAVE_USB_H && HAVE_PTHREADS_H */
   if(cameraName=="simulator") {
         cam = new QCamSimulator();
   } else {
      // find the best V4L device
      cam = QCamV4L2::openBestDevice(videoDeviceName.c_str());
      if (cam == NULL) {
         QMessageBox::information(0,"Qastrocam-g2","No camera detected\nSettings panel only");
         cout << "No camera detected" <<endl;
      }
   }

   // creating telescope object
   if(telescopeType=="none") telescopeType="";
   QTelescope * theTelescope=NULL;
   if (telescopeType.length() != 0) {
      if (telescopeType=="autostar") {
         theTelescope = new QTelescopeAutostar(telescopeDeviceName.c_str());
      } else if (telescopeType=="lx200") {
         theTelescope = new QTelescopeLX200(telescopeDeviceName.c_str());
      } else if (telescopeType=="nexstar") {
         theTelescope = new QTelescopeNexstar(telescopeDeviceName.c_str());
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
#if HAVE_USB_H && HAVE_PTHREADS_H
      } else if (telescopeType=="qhy5") {
         theTelescope = new QTelescopeQHY5();
      } else if (telescopeType=="qhy6") {
         theTelescope = new QTelescopeQHY6();
#endif /* HAVE_USB_H && HAVE_PTHREADS_H */
      } else if (telescopeType=="simulator") {
         theTelescope = new QTelescopeSimulator();
      } else {
         cerr << "Unsupported telescope type "
              <<"'"<<telescopeType<<"'"<<endl;
         cerr << "Your configuration file may be outdated. Please delete the file '.qastrocam.conf'." << endl;
         usage(argv[0]);
         exit(1);
      }
      theTelescope->buildGUI();
   }

   QKingClient* kingClient=NULL;
   QCamFindShift* findShift=NULL;
   QCamAutoGuidage* tracker=NULL;
   QCamAutoAlign* autoAlignCam=NULL;
   QCamFocus* camFocus=NULL;
   QCamTrans* camBias=NULL;
   QCamTrans* camDark=NULL;
   QCamTrans* camFlat=NULL;
   QCamTrans* camBayer=NULL;
   QCamTrans* camMirror=NULL;
   QCamTrans* camId=NULL;
   FrameMirror* mirrorAlgo=NULL;
   FrameBias* biasAlgo=NULL;
   FrameDark* darkAlgo=NULL;
   FrameFlat* flatAlgo=NULL;
   FrameBayer* bayerAlgo=NULL;
   FrameId* idAlgo=NULL;
   QCamStack* camStack=NULL;
   QCam* camAdd=NULL;
   QCam* camMax=NULL;

   if(cam!=NULL) {
      addRemoteCTRL(cam);
      cam->setCaptureFile("raw");
      QCam* camSrc=cam;

      // adding stack tab
      camStack=new QCamStack();

      // bias cam
      camBias = new QCamTrans();
      camBias->hideButtons(true);
      camBias->hideFile(true);
      camBias->hideMode(true);
      biasAlgo = new FrameBias(camBias);
      camBias->connectCam(*camSrc);
      camBias->connectAlgo(*biasAlgo);
      camStack->addCam(camBias,"Bias");
      camBias->setCaptureFile("bias");
      camSrc=camBias;

      // cam dark
      camDark = new QCamTrans();
      camDark->hideButtons(true);
      camDark->hideFile(true);
      camDark->hideMode(true);
      darkAlgo = new FrameDark(camDark);
      camDark->connectCam(*camSrc);
      camDark->connectAlgo(*darkAlgo);
      camStack->addCam(camDark,"Dark");
      camDark->setCaptureFile("dark");
      camSrc=camDark;

      // cam flat
      camFlat = new QCamTrans();
      camFlat->hideButtons(true);
      camFlat->hideFile(true);
      camFlat->hideMode(true);
      flatAlgo = new FrameFlat(camFlat);
      camFlat->connectCam(*camSrc);
      camFlat->connectAlgo(*flatAlgo);
      camStack->addCam(camFlat,"Flat");
      camFlat->setCaptureFile("bias");
      camSrc=camFlat;

      // cam bayer
      camBayer = new QCamTrans();
      camBayer->hideButtons(true);
      camBayer->hideFile(true);
      camBayer->hideMode(true);
      bayerAlgo = new FrameBayer(camBayer);
      camBayer->connectCam(*camSrc);
      camBayer->connectAlgo(*bayerAlgo);
      camStack->addCam(camBayer,"Bayer");
      camBayer->setCaptureFile("bayer");
      camSrc=camBayer;

      // mirror module
      camMirror = new QCamTrans();
      camMirror->hideButtons(true);
      camMirror->hideFile(true);
      camMirror->hideMode(true);
      mirrorAlgo = new FrameMirror(camMirror);
      camMirror->connectCam(*camSrc);
      camMirror->connectAlgo(*mirrorAlgo);
      camStack->addCam(camMirror,"Mirror");
      camMirror->setCaptureFile("mirror");
      camSrc=camMirror;

      // cam id
      camId = new QCamTrans();
      camId->hideMode(true);
      idAlgo = new FrameId(camId);
      camId->connectCam(*camSrc);
      camId->connectAlgo(*idAlgo);
      camStack->addCam(camId,"Calibrated");
      camId->setCaptureFile("calibrated");
      camSrc=camId;

      addRemoteCTRL(camStack);

      // King client object creation
      if (kingOption) {
         cout << "King aligment enabled\n";
         kingClient=new QKingClient();
         //sleep(1);
         kingClient->connectCam(*camSrc);
         //addRemoteCTRL(kingClient);
         kingClient->buildGUI(NULL)->show();
      }

      // alignement object
      if (theTelescope) {
         findShift=new QCamFindShift_hotSpot(theTelescope);
         findShift->connectCam(*camSrc);
         tracker = new QCamAutoGuidageSimple();
         tracker->setCam(camSrc);
         tracker->setTracker(findShift);
         tracker->setScope(theTelescope);
         //GUI build later
         //tracker->buildGUI();
      }
      // autoaligne creation
      if (autoAlign) {
         if(!findShift) {
            findShift=new QCamFindShift_hotSpot();
            findShift->connectCam(*camSrc);
         }
         autoAlignCam=new QCamAutoAlign();
         autoAlignCam->setTracker(findShift);
         addRemoteCTRL(autoAlignCam);
         camSrc=autoAlignCam;
      }

      if(tracker) {
         tracker->buildGUI();
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

      //creation du module focus
      //camFocus = new QCamFocus(camSrc);
      //addRemoteCTRL(camFocus);
   }

   // settings tab
   QSetting* settingsTab;
   settingsTab= new QSetting();
   addRemoteCTRL(settingsTab);

   // main window display
   QCamUtilities::setQastrocamIcon(&mainWidget);
   mainWindow.show();
   mainWindow.adjustSize();
   mainWindow.setFixedSize(mainWindow.size());

   // test settings version
   if(!settings.haveKey("CONF_VERSION")||QString(settings.getKey("CONF_VERSION"))!=_CONF_VERSION_) {
      QMessageBox::information(0,"Qastrocam-g2","Your configuration file is outdated. It should be deleted.\nDefault name is <.qastrocam-g2.conf>");
   }

   // QT event loop
   app.exec();

   // release all
   delete settingsTab;
   delete theTelescope;
   delete kingClient;
   delete findShift;
   delete tracker;
   delete autoAlignCam;
   delete camBias;
   delete camDark;
   delete camFlat;
   delete camBayer;
   delete camMirror;
   delete camId;
   delete biasAlgo;
   delete darkAlgo;
   delete flatAlgo;
   delete bayerAlgo;
   delete mirrorAlgo;
   delete idAlgo;
   delete camAdd;
   delete camMax;
   delete camStack;
   delete cam;

   QCamUtilities::removeWidget(&mainWidget);

#if HAVE_SDL_H
   SDL_Quit();
#endif

#ifndef _DEBUG_
   fclose(logFile);
#endif
   return(0);
}
