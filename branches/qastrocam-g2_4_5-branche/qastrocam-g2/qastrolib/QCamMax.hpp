#ifndef _QCamMax_hpp_
#define _QCamMax_hpp_

#include <qobject.h>


#include "QCam.hpp"
#include <qsize.h>

class QImage;

/** Stacks a serie of frame from .*/
class QCamMax : public QCam {
   Q_OBJECT
   QCam * cam_;
   void allocBuff(const QSize &);
   void addFrame(const uchar *);
   QCamFrame yuvFrame_;
public:
   QCamMax(QCam * cam);
   QCamFrame yuvFrame() const { return yuvFrame_; }
   const QSize & size() const { return cam_->size();}
   void resize(const QSize & s) {cam_->resize(s);}
   virtual const QSize * getAllowedSize() const { return QCam::getAllowedSize();}
   ~QCamMax() {};
   QWidget * buildGUI(QWidget * parent);
 public slots:
   void clear();
 private slots:
   void addNewFrame();
};

#endif
