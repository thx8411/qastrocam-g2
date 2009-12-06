#ifndef _QCamTrans_hpp_
#define _QCamTrans_hpp_

#include <qobject.h>
#include "QCam.hpp"

class FrameAlgo;

class QCamTrans : public QCam {
   Q_OBJECT
public:
   /** Activation mode */
   enum Mode {
      On=1 /** Frame from source are recieved and can be transformed. */,
      Off=2 /** Frame from source are no more recieved. */,
      Copy=3 /** Frame from source are passed untransformed. */
   };
   /** Default constructor */
   QCamTrans();
   /** to connect a new camera. */
   void connectCam(QCam & cam);
   /** disconnect the current camera */
   void disconnectCam();
   /** get the Mode.*/
   Mode mode() const {return mode_;}
   QCamFrame yuvFrame() const {return camFrame_; }
   void connectAlgo(FrameAlgo & algo);
   void disconnectAlgo();
   const QSize & size() const {
      if (cam_) {
         return cam_->size();
      } else return emptySize_;
   }
   void resize(const QSize & s) {
      if (cam_) {
         cam_->resize(s);
      }
   }
   virtual const QSize * getAllowedSize() const { return QCam::getAllowedSize();}
   virtual QWidget* buildGUI(QWidget * parent);
public slots:
   /** set the mode. */
   void mode(int mode);
signals:
   void modeChanged(int);
private slots:
   void transNewFrame();
private:
   QCam * cam_;
   bool paused_;
   QCamFrame camFrame_;
   Mode mode_;
   FrameAlgo * algo_;
   QWidget * algoWidget_;
   QSize emptySize_;
};

#endif
