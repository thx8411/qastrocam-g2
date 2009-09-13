#ifndef _QCam_hpp_
#define _QCam_hpp_

#include <qobject.h>

#include <string>

#include <stdio.h>
#include <stdlib.h>
//#include <linux/types.h>
//#include <linux/videodev.h>

#include <map>

#include "QCamFrame.hpp"
#include "Vector2D.hpp"

#include "../config.h"

class QWidget;
class QPushButton;
class QHBox;
class QImage;
class QLabel;
class QCamComboBox;
class QLCDNumber;
class QLineEdit;
class QTimer;
class QDirectoryChooser;
class QCamDisplay;
class CamHistogram;
class QGridBox;
class QCamMovie;

using namespace std;

/** base class of any object producing frames.
 */
class QCam : public QObject {
   Q_OBJECT
public:
   QCam();
   /** return the frame in yuv 420P format.
       it is the native format used by all algoithms.
   */
   virtual QCamFrame yuvFrame() const = 0;
   /** return the size of the current frame. */
   virtual const QSize & size() const =0;
   /** try to resize the current frame.
       it can failed. size() must be called to know the real
       new size. */
   virtual void resize(const QSize & s)=0;
   /** return an array of supported size of th capture window.
       The las entry of the table must be a null size (=QSize(0,0)).
       A minimal empty emplementation is given for no resizable QCam */
   virtual const QSize * getAllowedSize() const =0;
   virtual ~QCam();
   int getY(int x, int y) const {
      return yuvFrame().Y()[x+size().width()*y];
   }
   int getU(int x, int y) const {
      return yuvFrame().U()[x/2+size().width()*y/4];
   }
   int getV(int x, int y) const {
      return yuvFrame().V()[x/2+size().width()*y/4];
   }

   /** Set the value of a properties.
       Properties are dumped is file "<dirName>/properties.txt" in sequence capture mode.
       Properties are dumped is file "<fileName>.properties" in snapshot capture mode.
   */
   void setProperty(const string & prop, const string & val,
                    bool resetCapture=false);
   void setProperty(const string & prop, int val,
                    bool resetCapture=false);
   void setProperty(const string & prop, double val,
                    bool resetCapture=false);
   /** get the value of a properties.
    */
   string getProperty(const string & prop) const;
   /** import properties from an other QCam object. */
   void importProperties(const QCam & other);
   /** export properties */
   void exportProperties( map<string,string>& dest) const;
   /** get reference on properties */
   const map<string,string> & getProperties() const;
   /** build the GUI */
   virtual QWidget* buildGUI(QWidget * parent);
   /** indicate thet the GUI have been build */
   bool guiBuild() const { return remoteCTRL_ != NULL; }
   QWidget * gui() { return remoteCTRL_;}
   const QString & label() const;
   /** wrtite the properties associated to the camera */
   void writeProperties(const string & fileName) const;
   /** save the current frame with the current selected format */
   bool saveFrame(const string& file) const;
   /** put a small annotation on the frame */
   void annotate(const Vector2D & pos) const;
public slots:
   void snapshot() const;
   void setCapture(bool doCapture) const;
   void setPauseCapture(bool capturePaused) const;
   void displayFrames(bool display);
   void displayHistogram(bool display);
   bool capture() const {return doCapture_;}
   bool capturePaused() const {  return doCapture_ && capturePaused_;}
   void setCaptureFile(const QString & file);
protected:
   /** Must be called when a new YUV frame is
       avaible. It is used by frame() to opimize conversion
       from YUV to QImage.
   */
   void newFrameAvaible();
   void label(QString label);
   string getFileName() const;
   /** when called, will set the date of the frame to current date */
   void setTime();
private:
   QWidget * remoteCTRL_;
   void resetCaptureDir() const;
   mutable string seqenceFileName_;
   mutable bool doCapture_;
   mutable bool capturePaused_;
   string captureFile_;
   const char* getSaveFormat() const;
   const char ** fileFormatList_;
   int fileFormatCurrent_;
   int maxCaptureInSequence_;
   QGridBox * buttonBox_;
   QPushButton * displayFramesButton_;
   QCamDisplay * displayWindow_;
   QPushButton * displayHistogramButton_;
   CamHistogram * displayHistogramWindow_;
   QDirectoryChooser * dirChooser_;
   QCamComboBox * imgFormatBox_;
   QCamComboBox * sizeCombo;
   QPushButton * snapshot_;
   QPushButton * pauseCapture_;
   QPushButton * capture_;
   QLCDNumber * capturedFrame_;
   QLineEdit * fileNameW_;
   QLineEdit * maxCaptureInSequenceW_;
   QCamComboBox * periodicCaptureW_;
   int timebetweenCapture_;
   QLineEdit * timebetweenCaptureW_;
   QTimer * periodicCaptureT_;
   QHBox * buttons_;
   mutable string oldFileName_;
   mutable int fileSeqenceNumber_;
   string directory_;
   map<string,string> properties_;
   QString label_;
   mutable QCamMovie * movieWritter_;
   QCamMovie * movieWritterSeq_;
#if HAVE_AVIFILE_H
   QCamMovie * movieWritterAvi_;
#endif
 public:
   mutable Vector2D annotationPos_;
   mutable bool annotationEnabled_;
 signals:
   void newFrame();
 private slots:
   void updateFileFormat(int value);
   void maxCaptureInSequenceUpdated(const QString&);
   void timebetweenCaptureUpdated(const QString&);
   void periodicCapture(int mode);
   void timebetweenCaptureTimeout();
   void setDirectory(const QString & dir);
   void setSizeFromAllowed(int index);
};

#endif

