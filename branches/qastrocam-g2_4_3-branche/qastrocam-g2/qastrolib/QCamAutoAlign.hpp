#ifndef _QCamAutoAlign_hpp_
#define _QCamAutoAlign_hpp_

#include "QCam.hpp"
#include "ShiftInfo.hpp"
#include <stdio.h>
#include <string>

class QPushButton;
class QCamFindShift;
class ShiftInfo;
class QHistogram;
class QVectorMap;
class QCamSlider;
class QFrameDisplay;
class QHGroupBox;
class QCheckBox;

/** realign the frames of an other QCam object.
    It is usefull if the tracking is bad (or not existant).
*/
class QCamAutoAlign : public QCam {
   Q_OBJECT;


public:
   QCamAutoAlign();
   QCamFrame yuvFrame() const;
   void setTracker(QCamFindShift * tracker);
   virtual const QSize & size() const;
   virtual void resize(const QSize &s);
   virtual const QSize * getAllowedSize() const { return QCam::getAllowedSize();}
public slots:
   void reset();
protected slots:
   void shifted(const ShiftInfo & );
 private slots:
   void setCropValue(int crop) {cropValue_=crop/100.0;}
   void setImageCenter(bool center) {center_=center;}
private:
   QWidget * buildGUI(QWidget *);
   /** connect cam and tracker.
    */
   void shiftFrame(const ShiftInfo & shift,
                   const QCamFrame orig,
                   QCamFrame & shifted,
                   float crop,bool center);
   bool connectEveryThing();
   void unConnectEveryThing();
   QCamFindShift * tracker_;
   ShiftInfo currentShift_;
   QCamFrame yuvFrame_;
   //QFrameDisplay  * dispImgCenter_;
   QHGroupBox * findShiftWidget_;
   QWidget * findShiftCtrl_;
#define ONE_MAP 1
#if ONE_MAP
   QVectorMap * shiftMap_;
#else
   QHistogram * shiftXhisto_;
   QHistogram * shiftYhisto_;
#endif
   QCamSlider * scaleSlider_;
   QCheckBox * centerButton_;
   QCamSlider * cropSlider_;
   bool center_;
   float cropValue_;
   QSize currentSize_;
   string fifoName_;
   FILE * fifo_;
};
#endif
