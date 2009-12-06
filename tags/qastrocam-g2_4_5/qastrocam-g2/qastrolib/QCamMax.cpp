#include "QCamMax.moc"
#include "qpushbutton.h"

QCamMax::QCamMax(QCam* cam) {
   cam_=cam;
   connect(cam_,SIGNAL(newFrame()),this,SLOT(addNewFrame()));
   label("Max stacking");
   //initRemoteControl(remoteCTRL_);
}

void QCamMax::clear() {
   yuvFrame_.clear();
}

void QCamMax::addNewFrame() {
   QCamFrame origFrame = cam_->yuvFrame();
   bool colorMode=(origFrame.getMode()==YuvFrame);
   if (origFrame.size() != yuvFrame_.size()) {
      yuvFrame_.setSize(origFrame.size());
      yuvFrame_.clear();
   }
   int frameSize=yuvFrame_.size().height()*yuvFrame_.size().width();
   int frameWidth=yuvFrame_.size().width();
   uchar * locY=yuvFrame_.YforUpdate();
   uchar * locU=yuvFrame_.UforUpdate();
   uchar * locV=yuvFrame_.VforUpdate();
   const uchar * oriY=origFrame.Y();
   const uchar * oriU=NULL;
   const uchar * oriV=NULL;
   if (colorMode) {
      oriU=origFrame.U();
      oriV=origFrame.V();
   }
   int uvLineSize=frameWidth/2 /* 2/2 */;
   for(int i=0;i<frameSize;++i) {
      if (oriY[i] > locY[i]) {
         locY[i]=oriY[i];
         if (colorMode) {
            int x=i%frameWidth;
            int y=i/frameWidth;
            int shift2=(y/2)*uvLineSize+(x/2);
            locU[shift2]=oriU[shift2];
            locV[shift2]=oriV[shift2];
         }
      }
   }
   newFrameAvaible();
}

QWidget * QCamMax::buildGUI(QWidget * parent) {
   QWidget * remoteCTRL=QCam::buildGUI(parent);
   QPushButton * resetBufferFill_= new QPushButton("reset",remoteCTRL);
   connect(resetBufferFill_,SIGNAL(pressed()),this,SLOT(clear()));
   resetBufferFill_->show();
   return remoteCTRL;
}
