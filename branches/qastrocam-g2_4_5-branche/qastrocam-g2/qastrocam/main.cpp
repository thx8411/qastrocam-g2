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

// options strings
const string BrutDisplayString("-db");
const string PPortOptionString("-pc");
const string HistogramOptionString("-h");
const string AccumOptionString("-a");
const string AccumDisplayString("-da");
const string MirrorOptionString("-M");
const string MirrorDisplayString("-dM");
const string AutoAlignOptionString("-c");
const string AutoAlignDisplayString("-dc");
const string MaxOptionString("-m");
const string MaxDisplayString("-dm");
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
const string SDLon("--SDL");
const string SDLoff("--noSDL");
const string ExpertMode("--expert");
const string DeviceSource("-i");
const string DevicePalette("-p");
const string ForceGeneric("-df");
const string ForceSettings("-sf");

// back object, present everywhere
settingsBackup settings;

// qastrocam-g2 usage
void usage(const char * progName) {
   cerr << "usage: "<< progName<< " <options>"<<endl;
   cerr << "\nValid options are:"<<endl;
   cerr << "  "<<ForceSettings<<" to set the settings file name to use\n";
   cerr << "  "<<BrutDisplayString<<" to see the raw images from the cam\n";
   cerr << "  "<<HistogramOptionString<<" to see the histogram of the image from the cam and focus level info (needs '-b')\n";
   cerr << "  "<<MirrorOptionString<<" to swap left/right top/bottom of the image\n";
   cerr << "  "<<MirrorDisplayString<<" for options "<<MirrorOptionString
        << " diplay the swapped frames.\n";
   cerr << "  "<<AccumOptionString<<" to stack the images\n";
   cerr << "  "<<AccumDisplayString<<" for options "<<AccumOptionString
        << " display the frame after the stacking process\n";
   cerr << "  "<<MaxOptionString<<" to simulate very long exposure on fixed mount\n";
   cerr << "     It keeps the max of each pixel\n";
   cerr << "  "<<MaxDisplayString<<" for options "<<MaxOptionString
        << " display the frame after the stacking process\n";
   cerr << "  "<<AutoAlignOptionString<<" for options "<<AccumOptionString<<" & "<<MaxOptionString<<" align the frames before stacking\n";
   cerr << "  "<<AutoAlignDisplayString<<" for options "<<AutoAlignOptionString
        << " display the frame after the Align process\n";
   cerr << "  "<<VideoDeviceOptionString << " <deviceName> to choose the V4L device name.\n"
	<< "     default is /dev/video0.\n";
   cerr << "  "<<DeviceSource<< " <source> to set the V4L device source.\n";
   cerr << "  "<<DevicePalette<<" <palette> to force the V4L device palette.\n";
   cerr << "  "<<ForceGeneric<<" <yes/no> to force usage of V4L generic module.\n";
   cerr << "  "<<TelescopeTypeOption<<" <type> to select the telescope type\n"
	<< "     type 'help' will give the list of avaible telescope type\n";
   cerr << "  "<<TelescopeDeviceOptionString << " <deviceName> to choose the telescope control device or file.\n"
	<< "     default is /dev/ttyS1.\n";
   cerr << "  "<<LongexposureDeviceOptionString << " <deviceName> to choose de long exposure port (serial only).\n"
        << "     default is /dev/ttyS0 or /dev/parport0.\n";
   cerr << "  "<<LxLevelsInvertedOptionString<<" to invert polarity levels for serial and LED long exposure mods\n";
   cerr << "  "<<LxLevelsNormalOptionString<<" reset long exposure levels to non-inverted\n";
   cerr << "  "<<TsLevelsInvertedOptionString<<" to invert polarity levels for APM telescope\n";
   cerr << "  "<<TsLevelsNormalOptionString<<" reset APM levels to non-inverted\n";
   cerr << "  "<<LibDirOptionString<<" <directory> to set the library directory\n";
   cerr << "  "<<SDLon<<" use lib SDL to display frames (fast display).\n";
   cerr << "  "<<SDLoff<<" don't use lib SDL to display frames (slow display).\n";
   cerr << "  "<<KingOption<<" help to align the telescope with the king method.\n";
   cerr << "  "<<ExpertMode<<" enable some 'expert' options in the GUI\n";
   cerr << endl;
}

// ??
QTabWidget * getAllRemoteCTRL(QWidget * parent=0) {
   static QTabWidget * allRemote = new QTabWidget(parent,"allRemoteCTRL");
   return allRemote;
}

// ??
void addRemoteCTRL(QCam * cam) {
   getAllRemoteCTRL()->addTab(cam->buildGUI(getAllRemoteCTRL()),cam->label());
}

// ??
void addRemoteCTRL(QCamClient * client) {
   getAllRemoteCTRL()->addTab(client->buildGUI(getAllRemoteCTRL()),client->label());
}

int main(int argc, char ** argv) {
   int i;
   // default options values
   bool accum=false,max=false,mirror=false;
   bool brutDisplay=false,accumDisplay=false,maxDisplay=false,mirrorDisplay=false;
   bool autoAlign=false,autoAlignDisplay=false;
   bool histogram=false,telescope=false;
   bool kingOption=false;
   bool V4Lforce=false;
   string videoDeviceName("/dev/video0");
   string videoDeviceSource;
   string telescopeType;
   string telescopeDeviceName("/dev/ttyS1");
   string libPath;
   string settingsFileName(".qastrocam-g2-settings");

   cout << qastrocamName << " " << qastroCamVersion
        << " (build "<<qastrocamBuild<<")"<<endl;
   cout << "* based on " << QCamUtilities::getVersionId() <<endl;
   cout << "* " << qastrocamWeb << endl;
   cout << "* " << qastrocamMail << endl;

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
      if (BrutDisplayString == argv[i]) {
         brutDisplay=true;
      } else if (MirrorOptionString == argv[i]) {
         mirror=true;
      } else if (MirrorDisplayString == argv[i]) {
         mirrorDisplay=true;
         mirror=true;
      }  else if (AccumOptionString == argv[i]) {
         accum=true;
      } else if (AccumDisplayString == argv[i]) {
         accumDisplay=true;
         accum=true;
      } else if (MaxOptionString == argv[i]) {
         max=true;
      } else if (MaxDisplayString == argv[i]) {
         maxDisplay=true;
         max=true;
      } else if (HistogramOptionString == argv[i]) {
         histogram=true;
      } else if (AutoAlignOptionString == argv[i]) {
         autoAlign=true;
      } else if (AutoAlignDisplayString == argv[i]) {
         autoAlignDisplay =true;
         autoAlign=true;
      } else if (SDLon == argv[i]) {
         QCamUtilities::useSDL(true);
      } else if (SDLoff == argv[i]) {
         QCamUtilities::useSDL(false);
      } else if (ExpertMode == argv[i]) {
         QCamUtilities::expertMode(true);
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
      } else if ( DeviceSource == argv[i]) {
         i++;
         if(i==argc) {
            usage(argv[0]);
            exit(1);
         }
         videoDeviceSource=argv[i];
      } else if ( DevicePalette == argv[i]) {
         i++;
         if(i==argc) {
            usage(argv[0]);
            exit(1);
         }
         settings.setKey("PALETTE",argv[i]);
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
      cout << "supported scopes:\n"
           << "* apm\n"
           << "* autostar\n"
           << "* fifo\n"
           << "* mcu\n"
           << "* mts\n"
	   << "* file\n";
      exit(0);
   }

   // setting path
   if (libPath.empty()) {
      QCamUtilities::computePathName(argv[0]);
   } else {
      QCamUtilities::basePathName(libPath);
   }
   cout << "* lib directory "<<QCamUtilities::basePathName()<<"\n";
   cout << endl;

   // use SDL messages
   if (QCamUtilities::useSDL()) {
	   cout << "SDL display enabled. (If only a black windows is displayed,"
                << " try option "<<SDLoff<<" when launchnig qastrocam)\n";
   }

   // QT app settings
   QApplication app(argc,argv);
   QCamUtilities::setLocale(app);

   // main window setting
   QVBox mainWindow;
   QPushButton quit(&mainWindow,"Quit");
   QObject::connect( &quit, SIGNAL(released()), &app, SLOT(quit()) );
   quit.setPixmap(*QCamUtilities::getIcon("exit.png"));
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
   QCam  * cam =NULL;
   do {
      cam = QCamV4L::openBestDevice(videoDeviceName.c_str(),videoDeviceSource.c_str(),V4Lforce);
      if (cam == NULL) {
         static QMessageBox mb("Qastrocam-g2",
			      QMessageBox::tr("No camera detected (did you plug it?)"),
			      QMessageBox::Critical,
			      QMessageBox::Retry | QMessageBox::Default,
			      QMessageBox::Abort  | QMessageBox::Escape,
			      QMessageBox::NoButton);
         if ( mb.exec() == QMessageBox::Abort ) {
            cerr << "No camera detected" <<endl;
            exit(1);
         }
      }
   } while (cam==NULL);

   if (brutDisplay) {
      cam->displayFrames(true);
   }

   addRemoteCTRL(cam);
   cam->setCaptureFile("raw");
   QCam * camSrc=cam;

   // King client object creation
   QKingClient * kingClient=NULL;
   if (kingOption) {
      cout << "King aligment enabled\n";
      kingClient=new QKingClient();
      sleep(1);
      kingClient->connectCam(*camSrc);
      //addRemoteCTRL(kingClient);
      kingClient->buildGUI(NULL)->show();
   }

   // alignement object
   QCamFindShift * findShift=NULL;
   if (theTelescope || autoAlign) {
      //QCamFindShift * baryShift=new QCamFindShift_barycentre();
      findShift=new QCamFindShift_hotSpot(theTelescope);
      findShift->connectCam(*camSrc);
   }
   QCamAutoGuidage * tracker=NULL;
   if (theTelescope) {
      tracker = new QCamAutoGuidageSimple();
      tracker->setCam(camSrc);
      tracker->setTracker(findShift);
      tracker->setScope(theTelescope);
      //GUI build later
      //tracker->buildGUI();
   }

   // histogram creation
   if (histogram) {
      camSrc->displayHistogram(true);
      //CamHistogram * histo= new CamHistogram(*camSrc);
      //histo->show();
   }

   // autoaligne creation
   if (autoAlign) {
      assert(findShift);
      QCamAutoAlign * autoAlign=new QCamAutoAlign();
      autoAlign->setTracker(findShift);
      addRemoteCTRL(autoAlign);
      camSrc=autoAlign;

      if (autoAlignDisplay) {
         camSrc->displayFrames(true);
      }
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
      QCamTrans  * camMirror = new QCamTrans();
      FrameMirror * mirrorAlgo = new FrameMirror();
      camMirror->connectCam(*camSrc);
      camMirror->connectAlgo(*mirrorAlgo);
      addRemoteCTRL(camMirror);
      camMirror->setCaptureFile("mirror");
      camSrc=camMirror;

      if (mirrorDisplay) {
         camSrc->displayFrames(true);
      }
   }

   // accumulation module
   if (accum) {
      QCam  * camAdd = new QCamAdd(camSrc);
      addRemoteCTRL(camAdd);
      camAdd->setCaptureFile("add");
      camSrc=camAdd;

      if (accumDisplay) {
         camSrc->displayFrames(true);
      }
   }
   if (max) {
      //creation du module d'acumulation
      QCam  * camMax = new QCamMax(camSrc);
      addRemoteCTRL(camMax);
      camMax->setCaptureFile("max");
      camSrc=camMax;

      if (maxDisplay) {
         camSrc->displayFrames(true);
      }
   }

   // main window display
   QCamUtilities::setQastrocamIcon(&mainWindow);
   mainWindow.show();
   mainWindow.adjustSize();
   getAllRemoteCTRL()->show();
   getAllRemoteCTRL()->adjustSize();

   // QT event loop
   return app.exec();
}
