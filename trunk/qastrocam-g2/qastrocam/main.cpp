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
const string LevelsInvertedOptionString("--levels-inverted");
const string LevelsNormalOptionString("--levels-normal");
const string LibDirOptionString("--libdir");
const string SDLon("--SDL");
const string SDLoff("--noSDL");
const string ExpertMode("--expert");
const string DeviceSource("-i");

extern string longexposureDeviceName;

settingsBackup settings;

void usage(const char * progName) {
   cerr << "usage: "
        << progName
        << " <options>"<<endl;
   cerr << "\nValid options are:"<<endl;
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
   cerr << "  "<<PPortOptionString << " <port> IO port of the // port (in Hexa).\n"
	<< "     default is 378 (=LPT1) (use 278 for LPT2).\n"
        << "     * Only for APM interface *\n";
   cerr << "  "<<TelescopeTypeOption<<" <type> to select the telescope type\n"
	<< "     type 'help' will give the list of avaible telescope type\n";
   cerr << "  "<<TelescopeDeviceOptionString << " <deviceName> to choose the telescope serial port control.\n"
	<< "     default is /dev/ttyS0.\n";
   cerr << "  "<<LongexposureDeviceOptionString << " <deviceName> to choose de long exposure port (serial only).\n"
        << "     default is /dev/ttyS1.\n";
   cerr << "  "<<LevelsInvertedOptionString<<" to invert polarity levels for serial and LED SCmods\n";
   cerr << "  "<<LevelsNormalOptionString<<" reset levels to non-inverted\n";
   cerr << "  "<<LibDirOptionString<<" <directory> to set the library directory\n";
   cerr << "  "<<SDLon<<" use lib SDL to display frames (fast display).\n";
   cerr << "  "<<SDLoff<<" don't use lib SDL to display frames (slow display).\n";
   cerr << "  "<<KingOption<<" help to align the telescope with the king method.\n";
   cerr << "  "<<ExpertMode<<" enable some 'expert' options in the GUI\n";
   cerr << endl;
}

QTabWidget * getAllRemoteCTRL(QWidget * parent=0) {
   static QTabWidget * allRemote = new QTabWidget(parent,"allRemoteCTRL");
   return allRemote;
}

void addRemoteCTRL(QCam * cam) {
   getAllRemoteCTRL()->addTab(cam->buildGUI(getAllRemoteCTRL()),cam->label());
}

void addRemoteCTRL(QCamClient * client) {
   getAllRemoteCTRL()->addTab(client->buildGUI(getAllRemoteCTRL()),client->label());
}

int main(int argc, char ** argv) {

   bool accum=false,max=false,mirror=false;
   bool brutDisplay=false,accumDisplay=false,maxDisplay=false,mirrorDisplay=false;
   bool autoAlign=false,autoAlignDisplay=false;
   bool histogram=false,telescope=false;
   bool kingOption=false;
   string videoDeviceName("/dev/video0");
   string videoDeviceSource;
   string telescopeType;
   string telescopeDeviceName("/dev/ttyS0");
   string libPath;
   
   int pportNumber=0x378;
   PPort * paralPort=NULL;
   
   cout << qastrocamName << " " << qastroCamVersion
        << " (build "<<qastrocamBuild<<")"<<endl;
   cout << "* based on " << QCamUtilities::getVersionId() <<endl;
   cout << "* " << qastrocamWeb << endl;
   cout << "* " << qastrocamMail << endl;
   
   settings.deSerialize();

   if(settings.haveKey("TELESCOPE_DEVICE")) telescopeDeviceName=settings.getKey("TELESCOPE_DEVICE");
   if(settings.haveKey("LX_DEVICE")) longexposureDeviceName=settings.getKey("LX_DEVICE");

   for (int i=1;i <argc;++i) {
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
         longexposureDeviceName=argv[i];
         settings.setKey("LX_DEVICE",argv[i]);
      } else if ( LevelsInvertedOptionString == argv[i]) {
         settings.setKey("LX_LEVELS_INVERTED","yes");
      } else if ( LevelsNormalOptionString == argv[i]) {
         settings.setKey("LX_LEVELS_INVERTED","no");
      } else if ( PPortOptionString == argv[i]) {
         ++i;
         if (i==argc) {
            usage(argv[0]);
            exit(1);
         }
         sscanf(argv[i],"%x",&pportNumber);
         cout << "using // port 0x"<<hex<<pportNumber<<dec<<endl;
      } else if ( LibDirOptionString == argv[i]) {
         ++i;
         if (i==argc) {
            usage(argv[0]);
            exit(1);
         }
         libPath=argv[i];
      } else if ( KingOption == argv[i]) {
         kingOption=true;
      } else {
         cerr << "Invalid option '"<<argv[i]<<"'"<<endl;
         usage(argv[0]);
         exit(1);
      }
   }

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

   if (libPath.empty()) {
      QCamUtilities::computePathName(argv[0]);
   } else {
      QCamUtilities::basePathName(libPath);
   }
   cout << "* lib directory "<<QCamUtilities::basePathName()<<"\n";
   cout << endl;
  
   if (QCamUtilities::useSDL()) {
	   cout << "SDL display enabled. (If only a black windows is displayed,"
                << " try option "<<SDLoff<<" when launchnig qastrocam)\n";
   }
   QApplication app(argc,argv);
   
   QCamUtilities::setLocale(app);

   QVBox mainWindow;
   QPushButton quit(&mainWindow,"Quit");
   QObject::connect( &quit, SIGNAL(released()), &app, SLOT(quit()) );
   quit.setPixmap(*QCamUtilities::getIcon("exit.png"));
   app.setMainWidget(&mainWindow);
   getAllRemoteCTRL(&mainWindow);
      
   /*
   if (!(brutDisplay || accumDisplay
         || telescope || maxDisplay || autoAlignDisplay
         || mirror)) {
      usage(argv[0]);
      exit(1);
   }
   */
   QTelescope * theTelescope=NULL;
   
   if (telescopeType.length() != 0) {
      if (telescopeType=="autostar") {
         theTelescope = new QTelescopeAutostar(telescopeDeviceName.c_str());
      } else if (telescopeType=="mcu") {
         theTelescope = new QTelescopeMCU(telescopeDeviceName.c_str());
      } else if (telescopeType=="apm") {
         paralPort=PPort::getPPort(pportNumber);
         theTelescope = new QTelescopeAPM(paralPort);
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

   // creation du module d'acquisition
   //QCamVesta cam("/dev/video0");
   QCam  * cam =NULL;
	   
   do {
      cam = QCamV4L::openBestDevice(videoDeviceName.c_str(),videoDeviceSource.c_str());
      if (cam == NULL) {
         static QMessageBox mb("Qastrocam",
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

   QKingClient * kingClient=NULL;
   
   if (kingOption) {
      cout << "King aligment enabled\n";
      kingClient=new QKingClient();
      sleep(1);
      kingClient->connectCam(*camSrc);
      //addRemoteCTRL(kingClient);
      kingClient->buildGUI(NULL)->show();
   }
   

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
      /*
        GUI build later
        tracker->buildGUI();
      */
   }

   if (histogram) {
      camSrc->displayHistogram(true);
      //CamHistogram * histo= new CamHistogram(*camSrc);
      //histo->show();
   }

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
   
   if (mirror) {
      //mirror module
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

   if (accum) {
      //creation du module d'acumulation
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
   QCamUtilities::setQastrocamIcon(&mainWindow);

   //mainWindow.move(0,0);
   mainWindow.show();
   mainWindow.adjustSize();

   getAllRemoteCTRL()->show();
   getAllRemoteCTRL()->adjustSize();
   
   return app.exec();
}
