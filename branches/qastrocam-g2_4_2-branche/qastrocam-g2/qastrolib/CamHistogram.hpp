#ifndef _CamHistogram_hpp_
#define _CamHistogram_hpp_

#include <qpainter.h>
#include <qimage.h>
//#include <qhgroupbox.h>

#include <stdio.h>
#include <stdlib.h>
//#include <linux/types.h>
//#include <linux/videodev.h>

#include "QCamClient.hpp"

class QPushButton;
class QWidget;
class QVGroupBox;
class QProgressBar;
class QLCDNumber;
class QHBox;
class QHGroupBox;
class QLabel;
class QHistogram;
class QCam;

#define focusHistorySize_ 200

/** Helper windows to display an histogram. */
class CamHistogram : public QCamClient {
   Q_OBJECT
public:
   CamHistogram(QCam &);
   QWidget & widget();
   const QWidget & widget() const;
protected slots:
   void newFrame();
private:
   QHGroupBox * mainWindow_;
   QVGroupBox * histoGroup_;
   QHistogram * histogramArea_;
   QVGroupBox * focusGroup_;
   
   QPushButton * resetFocus_;
   QWidget * focusLevel_;
   QHistogram * focusArea_;
   QHBox * seeingGroup_;
   QLabel * seeingLabel_;
   QProgressBar * seeingLevel_;
   //QLCDNumber * seeingValue_;
   void init();
   //QCam * cam_;
   int histogram_[256];
   int focusIndexValue_;
   double getDistFromNeibourg(int x,int y) const;
};

#endif
