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


#include "QCam.moc"
#include <iostream>

#include <qvbox.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <qimage.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qhgroupbox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlcdnumber.h>
#include <qlineedit.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qiconset.h>
#include <qstringlist.h>
#include "QCamRadioBox.hpp"
#include "QCamComboBox.hpp"
#include "QCamUtilities.hpp"
#include "QDirectoryChooser.hpp"
#include "QCamDisplay.hpp"
#include "CamHistogram.hpp"
#include <stdio.h>
#include "QGridBox.hpp"
#include "FitsImage.hpp"
#include "QCamMovieAvi.hpp"
#include "QCamMovieAviLossless.hpp"
#include "QCamMovieSeq.hpp"
#include "SettingsBackup.hpp"

#include <sys/time.h>

extern settingsBackup settings;

QCam::QCam() {
   doCapture_=false;
   capturePaused_=false;
   captureFile_="qcam";
   fileSeqenceNumber_=0;
   maxCaptureInSequence_=10;
   timebetweenCapture_=60;
   croppingMode=0;
   sizeTable=NULL;
   remoteCTRL_=NULL;
   snapshot_=NULL;
   directory_=".";
   displayWindow_=NULL;
   displayFramesButton_=NULL;
   displayHistogramWindow_=NULL;
   displayHistogramButton_=NULL;
   sizeCombo=NULL;
   cropCombo=NULL;
   //initRemoteControl();
   //remoteCTRL_->show();
#if HAVE_AVIFILE_H
   movieWritterAvi_=new QCamMovieAvi();
   movieWritterAviLossless_=new QCamMovieAviLossless();
#endif
   movieWritterSeq_=new QCamMovieSeq();
   movieWritter_=NULL;
   annotationEnabled_=false;
}

QCam::~QCam() {
   delete periodicCaptureT_;
   free(fileFormatList_);
   delete movieWritterAvi_;
   delete movieWritterAviLossless_;
   delete movieWritterSeq_;
   delete displayWindow_;
   delete displayHistogramWindow_;
}

const QSize * QCam::getAllowedSize() const {
   static QSize empty(0,0);
   return & empty;
}

void QCam::setDirectory(const QString & dir) {
   directory_=dir.latin1();
}

const char * QCam::getSaveFormat() const {
   return fileFormatList_[fileFormatCurrent_];
}

bool QCam::saveFrame(const string& file) const {
   writeProperties(file+".properties");

   string fileFormat=getSaveFormat();
   int quality=-1;
   if (fileFormat=="FITS") {
      FitsImageCFITSIO fit(file+".fit");
            return fit.save(yuvFrame());
   } else
      return yuvFrame().colorImage().save((file
                                           +"."
                                           +QString(fileFormat).lower().latin1()).c_str(),
                                          fileFormat.c_str(), quality);
}

void QCam::snapshot() const {
   string fileName=directory_+"/"+captureFile_+"-"+getFileName();
   mkdir(directory_.c_str(),0777);
   //writeProperties(fileName+".properties");
   if (!saveFrame(fileName)) {
      cerr << "can't save file "<<fileName;
   }
}

void QCam::resetCaptureDir() const {
   fileSeqenceNumber_=0;
   seqenceFileName_=directory_+"/"+captureFile_+"-"+getFileName();
   mkdir(directory_.c_str(),0777);
   mkdir(seqenceFileName_.c_str(),0777);
   //writeProperties(seqenceFileName_+"/properties.txt");
   capturedFrame_->display(0);
}

void QCam::setCapture(bool doCapture) const {
   if (doCapture == doCapture_) {
      // no changes
      return;
   }
   fileSeqenceNumber_=0;
   if (doCapture) {
      // default writer
      movieWritter_=movieWritterSeq_;
#if HAVE_AVIFILE_H
      string fileFormat=getSaveFormat();
      if (fileFormat=="AVI raw") {
         movieWritter_=movieWritterAvi_;
      } else if (fileFormat=="AVI huff") {
         movieWritter_=movieWritterAviLossless_;
      }
#endif
              string fileName=directory_+"/"+captureFile_+"-"+getFileName();
      movieWritter_->open(fileName,*this);
   } else {
      movieWritter_->close();
   }
   capturedFrame_->display(fileSeqenceNumber_);
   doCapture_=doCapture;
   capturedFrame_->setDisabled(!doCapture);
   capture_->setOn(doCapture);
   pauseCapture_->setDisabled(!doCapture);
   setPauseCapture(false);
}

void QCam::setPauseCapture(bool capturePaused) const {
   capturePaused_=capturePaused;
   pauseCapture_->setOn(capturePaused);
}

void QCam::setCaptureFile(const QString & afile) {
   string file=afile.latin1();
   if (file.size() !=0) {
      captureFile_=file;
      if (guiBuild()) {
         QToolTip::add(snapshot_,
                       (string("Save snapshot image in file '")
                        + captureFile_ +"-<date>.<type>'").c_str());
#if HAVE_AVIFILE_H
         QToolTip::add(capture_,
                       (string("Save sequence in raw/lossless AVI file or picture sequence '")
                        + captureFile_ +"-<date>.<type>'").c_str());
#else
         QToolTip::add(capture_,
                       (string("Save sequence in directory '")
                        + captureFile_ +"-<date>'").c_str());
#endif
      }
      if (guiBuild()) {
         fileNameW_->setText(captureFile_.c_str());
      }
   } else
      captureFile_="NoName";
}

void QCam::newFrameAvaible() {
   //if (getProperty("TIME")=="N/A") {
      setTime();
   //}
   yuvFrame().setAllProperies(properties_);
   if (doCapture_ && !capturePaused_) {
      if (movieWritter_->add(yuvFrame(),*this)) {
         ++fileSeqenceNumber_;
      }
      if (fileSeqenceNumber_>9999) {
         capturedFrame_->setNumDigits(5);
      } else if (fileSeqenceNumber_>999) {
         capturedFrame_->setNumDigits(4);
      } else if (fileSeqenceNumber_>99) {
         capturedFrame_->setNumDigits(3);
      }
      capturedFrame_->display(fileSeqenceNumber_);
      if ((fileSeqenceNumber_+1) > maxCaptureInSequence_) {
         //capture_->toggle();
         setCapture(false);
      }
   }
   emit newFrame();
}

const QString & QCam::label() const {
   return label_;
}

void QCam::label(QString label) {
   label_=label;
}

void QCam::displayFrames(bool val) {
   if (displayFramesButton_ && displayFramesButton_->isOn() != val) {
      displayFramesButton_->setOn(val);
      return;
   }
   if (val) {
      if (displayWindow_ == NULL) {
         displayWindow_ = new QCamDisplay();
         displayWindow_->connectCam(*this);
      }
      displayWindow_->resume();
      displayWindow_->widget().show();
   } else {
      if (displayWindow_) {
         displayWindow_->pause();
         displayWindow_->widget().hide();
         delete displayWindow_;
         displayWindow_=NULL;
      }
   }
}

void QCam::displayHistogram(bool val) {
   if (displayHistogramButton_ && displayHistogramButton_->isOn() != val) {
      displayHistogramButton_->setOn(val);
      return;
   }
   if (displayHistogramWindow_ == NULL) {
      displayHistogramWindow_ = new CamHistogram(*this);
      displayHistogramWindow_->widget().setCaption(label());
   }
   if (val) {
      displayHistogramWindow_->resume();
      displayHistogramWindow_->widget().show();
   } else {
      displayHistogramWindow_->pause();
      displayHistogramWindow_->widget().hide();
   }
}

QWidget * QCam::buildGUI(QWidget * parent) {
   QPixmap* tmpIcon;

   QSizePolicy sizePolicyMin;
   sizePolicyMin.setVerData(QSizePolicy::Minimum);
   sizePolicyMin.setHorData(QSizePolicy::Minimum);

   remoteCTRL_= new QVBox(parent);

   remoteCTRL_->setSizePolicy(sizePolicyMin);

   buttonBox_= new QGridBox(remoteCTRL_,Qt::Vertical,4);
   displayFramesButton_ = new QPushButton("display",buttonBox_);
   QToolTip::add(displayFramesButton_,"display frames");
   displayFramesButton_->setToggleButton(true);
   tmpIcon=QCamUtilities::getIcon("displayFrames.png");
   displayFramesButton_->setPixmap(*tmpIcon);
   delete tmpIcon;
   if (displayWindow_ && displayWindow_->isActive()) {
      cout << "set displayFramesButton_ on\n";
      displayFramesButton_->setOn(true);
   }
   connect(displayFramesButton_,SIGNAL(toggled(bool)),this,SLOT(displayFrames(bool)));

   displayHistogramButton_ = new QPushButton("histo",buttonBox_);
   QToolTip::add(displayHistogramButton_,"display frames histograms,\nand focus information");
   displayHistogramButton_->setToggleButton(true);
   tmpIcon=QCamUtilities::getIcon("displayHistogram.png");
   displayHistogramButton_->setPixmap(*tmpIcon);
   delete tmpIcon;
   displayHistogramButton_->setMinimumHeight(28);
   if (displayHistogramWindow_ && displayHistogramWindow_->isActive()) {
      cout << "set displayHistogramButton_ on\n";
      displayHistogramButton_->setOn(true);
   }
   connect(displayHistogramButton_,SIGNAL(toggled(bool)),this,SLOT(displayHistogram(bool)));

   QStringList formatList =QImage::outputFormatList();

   int size=0;
   int tmpTab[16];

   fileFormatList_=(const char**)malloc(sizeof(const char*)*16);
   // adds fits format
   fileFormatList_[size]="FITS";
   tmpTab[size]=size;
   ++size;
   // adds fits-gz format
   //fileFormatList_[size]="FITS-GZ";
   //tmpTab[size]=size;
   //++size;
#if HAVE_AVIFILE_H
   // adds avi format
   fileFormatList_[size]="AVI raw";
   tmpTab[size]=size;
   ++size;
   // adds avi huff format
   fileFormatList_[size]="AVI huff";
   tmpTab[size]=size;
   ++size;
#endif
   // adds bmp format
   if(formatList.findIndex("BMP")!=-1) {
      fileFormatList_[size]="BMP";
      tmpTab[size]=size;
      ++size;
   }
   // adds png format
   if(formatList.findIndex("PNG")!=-1) {
      fileFormatList_[size]="PNG";
      tmpTab[size]=size;
      ++size;
   }
   // default format
   fileFormatCurrent_=0;
   // looking for the settings stored format
   if(settings.haveKey("FILE_FORMAT")) {
      for(int i=0;i<size;++i) {
         if (!strcasecmp(fileFormatList_[i],settings.getKey("FILE_FORMAT"))) {
            fileFormatCurrent_=i;
            break;
         }
      }
   // else looking for bmp or png format
   } else {
      for(int i=0;i<size;++i) {
         if (!strcasecmp(fileFormatList_[i],"PNG")) {
            fileFormatCurrent_=i;
            // png is good, not trying to find a better one
            // save setting
            settings.setKey("FILE_FORMAT","PNG");
            break;
         } else if (!strcasecmp(fileFormatList_[i],"BMP")) {
            fileFormatCurrent_=i;
            // save setting
            settings.setKey("FILE_FORMAT","BMP");
         }
      }
   }

   QVGroupBox * saveGroup = new QVGroupBox("Save Images",remoteCTRL_);
   buttons_=new QHBox(saveGroup);

   new QLabel("Prefix:",buttons_);
   fileNameW_ = new  QLineEdit(buttons_);
   //fileNameW_->setMaxLength(10);
   fileNameW_->setText(captureFile_.c_str());
   connect(fileNameW_,SIGNAL(textChanged(const QString&)),
           this,SLOT(setCaptureFile(const QString&)));
   QToolTip::add(fileNameW_,
                 "prefix used for saved images");
   dirChooser_ = new QDirectoryChooser(buttons_);
   connect(dirChooser_,SIGNAL(directoryChanged(const QString &)),this,SLOT(setDirectory(const QString &)));

   imgFormatBox_ = new QCamComboBox("Save Format",buttons_,
                                    size,tmpTab,
                                    fileFormatList_);
   connect(imgFormatBox_,SIGNAL(change(int)),
           this,SLOT(updateFileFormat(int)));
   imgFormatBox_->update(fileFormatCurrent_);

   QHBox * buttons2=new QHBox(saveGroup);
   snapshot_=new QPushButton("snapshot",buttons2);
   tmpIcon=QCamUtilities::getIcon("snapshot.png");
   snapshot_->setPixmap(*tmpIcon);
   delete tmpIcon;
   capture_=new QPushButton("capture",buttons2);
   capture_->setToggleButton(true);
   tmpIcon=QCamUtilities::getIcon("movie.png");
   capture_->setPixmap(*tmpIcon);
   delete tmpIcon;

   pauseCapture_=new QPushButton("pause",buttons2);
   QToolTip::add(pauseCapture_,
                 "Suspend the current capture");
   pauseCapture_->setToggleButton(true);
   tmpIcon=QCamUtilities::getIcon("movie_pause.png");
   pauseCapture_->setPixmap(*tmpIcon);
   delete tmpIcon;
   capturedFrame_=new QLCDNumber(buttons2);
   capturedFrame_->setSegmentStyle(QLCDNumber::Flat);
   capturedFrame_->setNumDigits(2);
   capturedFrame_->show();
   capturedFrame_->setDisabled(true);

   maxCaptureInSequenceW_=new QLineEdit(buttons2);
   maxCaptureInSequenceW_->setMaxLength(4);
   maxCaptureInSequenceW_->
      setText(QString().sprintf("%d",maxCaptureInSequence_));
   connect(maxCaptureInSequenceW_,SIGNAL(textChanged(const QString&)),
           this,SLOT(maxCaptureInSequenceUpdated(const QString&)));
   QToolTip::add(maxCaptureInSequenceW_,"Number of frames to capture");

   maxCaptureInSequenceW_->show();

   connect(snapshot_,SIGNAL(released()),this,SLOT(snapshot()));
   connect(capture_,SIGNAL(toggled(bool)),this,SLOT(setCapture(bool)));
   connect(pauseCapture_,SIGNAL(toggled(bool)),this,SLOT(setPauseCapture(bool)));

   buttons_->show();
   buttons2->show();
   snapshot_->show();
   capture_->show();

   if((getSaveFormat()=="AVI raw")||(getSaveFormat()=="AVI huff"))
      snapshot_->setEnabled(FALSE);
   else
      snapshot_->setEnabled(TRUE);

   setCaptureFile(captureFile_.c_str());

   /* periodic capture */
   QHGroupBox * savePeriodicGroup = new QHGroupBox("Periodic capture",
                                                   saveGroup);
   int valueList[]={0,1,2};
   const char* labelList[] = {"none","snap.","sequ."};
   periodicCaptureW_ =
      new QCamComboBox("Periodic capture",savePeriodicGroup,3,valueList,
                       labelList);
   connect(periodicCaptureW_,SIGNAL(change(int)),
           this,SLOT(periodicCapture(int)));

   timebetweenCaptureW_ = new QLineEdit(savePeriodicGroup);
   QToolTip::add(timebetweenCaptureW_,"Time between two automatic capture");
   timebetweenCaptureW_->
      setText(QString().sprintf("%d",timebetweenCapture_));
   connect(timebetweenCaptureW_,SIGNAL(textChanged(const QString&)),
           this,SLOT(timebetweenCaptureUpdated(const QString&)));
   timebetweenCaptureW_->setMaxLength(4);
   //timebetweenCaptureW_->hide();
   periodicCaptureW_->update(0);
   periodicCaptureW_->show();
   periodicCaptureT_=new QTimer();
   connect(periodicCaptureT_,SIGNAL(timeout()),
           this,SLOT(timebetweenCaptureTimeout()));
   periodicCaptureT_->start(timebetweenCapture_*1000);

   setCapture(false);

   if (sizeTable && !sizeTable[0].isEmpty()) {
      QHGroupBox* sizeGroup= new QHGroupBox("Resizing",remoteCTRL_);

      int size=0;
      while (!sizeTable[size].isEmpty())
         size++;
      char** labelList=(char**)malloc(size*sizeof(char*));
      int* valueList=(int*)malloc(size*sizeof(int));
      int indexOfCurrentSize=-1;
      for(int i=0;i<size;++i) {
         char tmpBuff[15];
         sprintf(tmpBuff,"%dx%d",sizeTable[i].width(),sizeTable[i].height());
         labelList[i]=strdup(tmpBuff);
         valueList[i]=i;
         if (this->size()==sizeTable[i]) {
            indexOfCurrentSize=i;
         }
      }

      //QLabel* l1=new QLabel("Size :",sizeGroup);
      sizeCombo=new QCamComboBox("Frame size",sizeGroup,size,valueList,(const char**)labelList);
      for(int i=0;i<size;i++)
         free(labelList[i]);
      free(labelList);
      free(valueList);
      // looking for settings stored frame resolution
      if(settings.haveKey("FRAME_RESOLUTION"))
         indexOfCurrentSize=sizeCombo->getPosition(settings.getKey("FRAME_RESOLUTION"));

      if (indexOfCurrentSize==-1) {
         cout << "warning : current capture size not found" << endl;
         cout << "Using default..." << endl;
         indexOfCurrentSize=0;
      }
      sizeCombo->update(indexOfCurrentSize);
      setSizeFromAllowed(indexOfCurrentSize);
      connect(sizeCombo,SIGNAL(change(int)),this,SLOT(setSizeFromAllowed(int)));
      QToolTip::add(sizeCombo,"Frame size");

      // resizing mode combo
      const char* labelList2[]={"Scaling","Cropping","Binning"};
      int valueList2[]={SCALING,CROPPING,BINNING};
      cropCombo=new QCamComboBox("Cropping mode",sizeGroup,3,valueList2,labelList2);
      connect(cropCombo,SIGNAL(change(int)),this,SLOT(setCropping(int)));
      QToolTip::add(cropCombo,"Resizing mode");
   }
   return remoteCTRL_;
}

string QCam::getFileName() const {
   char buff[30];
   time_t timet;
   time(&timet);
   struct tm * t=gmtime(&timet);
   snprintf(buff,30,"%04d.%02d.%02d-%02dh%02dm%02ds",
            t->tm_year+1900,t->tm_mon+1,t->tm_mday,
            t->tm_hour,t->tm_min,t->tm_sec);
   return buff;
}

void QCam::importProperties(const QCam & other) {
   for(map<string,string>::const_iterator it=other.properties_.begin();
       it !=other.properties_.end();
       ++it) {
      properties_[it->first]=it->second;
   }
}

void QCam::setProperty(const string & prop, const string  & val,bool resetCapture) {
   map<string,string>::iterator it=properties_.find(prop);
   if (it == properties_.end() || it->second != val) {
      properties_[prop]=val;
      if (doCapture_ && resetCapture) {
         setCapture(false);
         setCapture(true);
      }
   }
}

void QCam::setProperty(const string & prop, int val,bool resetCapture) {
   char buff[30];
   snprintf(buff,30,"%5d",val);
   setProperty(prop,buff,resetCapture);
}

void QCam::setProperty(const string & prop, double val,bool resetCapture) {
   char buff[30];
   snprintf(buff,30,"%5.3f",val);
   setProperty(prop,buff,resetCapture);
}

string QCam::getProperty(const string & prop) const {
   static string empty="N/A";
   map<string,string>::const_iterator it=properties_.find(prop);

   if (it != properties_.end()) {
      return it->second;
   } else {
      return empty;
   }
}

const map<string,string> & QCam::getProperties() const {
   return properties_;
}

void QCam::writeProperties(const string & fileName) const {
   FILE * file=fopen(fileName.c_str(),"w");
   if (file == NULL) {
      cerr << "can't write file " << fileName <<endl;
      return;
   }
   for (map<string,string>::const_iterator it=properties_.begin();
        it!=properties_.end();
        ++it) {
      fprintf(file,"%s=%s\n",it->first.c_str(),it->second.c_str());
   }
   fclose(file);
}

void QCam::updateFileFormat(int value) {
   fileFormatCurrent_=value;

   if((getSaveFormat()=="AVI raw")||(getSaveFormat()=="AVI huff"))
      snapshot_->setEnabled(FALSE);
   else
      snapshot_->setEnabled(TRUE);

   // saving file format
   settings.setKey("FILE_FORMAT",getSaveFormat());
}

void QCam::maxCaptureInSequenceUpdated(const QString &newMaxStr) {
   int newMax;
   bool res;
   newMax=newMaxStr.toUInt(&res);
   if (newMax > 0 && res) {
      maxCaptureInSequence_=newMax;
   } else if (newMaxStr.isEmpty()) {
      maxCaptureInSequence_=0;
   }
   maxCaptureInSequenceW_->
      setText(QString().sprintf("%d",maxCaptureInSequence_));
}

 void QCam::periodicCapture(int mode) {

   if (mode != 0) {
      //timebetweenCaptureW_->show();
      periodicCaptureT_->changeInterval(timebetweenCapture_*1000);
      timebetweenCaptureTimeout();
   } else {
      //timebetweenCaptureW_->hide();
   }
}

void QCam::timebetweenCaptureUpdated(const QString &newMaxStr) {
   int newMax;
   bool res;
   newMax=newMaxStr.toUInt(&res);
   if (newMax > 0 && res) {
      timebetweenCapture_=newMax;
   }
   timebetweenCaptureW_->
      setText(QString().sprintf("%d",timebetweenCapture_));
   periodicCaptureT_->changeInterval(timebetweenCapture_*1000);
   //cout << "timebetweenCapture_ changed to "<<timebetweenCapture_<<endl;
}

void  QCam::timebetweenCaptureTimeout() {
   //cout << "timebetweenCaptureTimeout "<< periodicCaptureW_->value() << endl;

   switch(periodicCaptureW_->value()) {
   case 0:
      break;
   case 1:
      // snapshot
      snapshot();
      break;
   case 2:
      setCapture(true);
      break;
   }
}

void QCam::setTime() {
   struct tm TM;
   struct timeval TIMEVAL;
   char buff[100];
   gettimeofday(&TIMEVAL,NULL);
   gmtime_r(&TIMEVAL.tv_sec,&TM);
   snprintf(buff,100,"%04d-%02d-%02d-%02d-%02d-%02d.%06ld",
            TM.tm_year+1900,TM.tm_mon+1,TM.tm_mday,
            TM.tm_hour, TM.tm_min, TM.tm_sec,
            TIMEVAL.tv_usec
      );
   setProperty("TIME",buff);
}

void QCam::annotate(const Vector2D & pos) const {
   annotationEnabled_=true;
   annotationPos_=pos;
}

void QCam::setSizeFromAllowed(int index) {
   // saving frame resolution
   settings.setKey("FRAME_RESOLUTION",sizeCombo->text(index));
   if(cropCombo!=NULL)
      croppingMode=cropCombo->value();
   resize(sizeTable[index]);
}

void QCam::setCropping(int index) {
   croppingMode=index;
   if(sizeCombo!=NULL)
      resize(sizeTable[sizeCombo->value()]);
}
