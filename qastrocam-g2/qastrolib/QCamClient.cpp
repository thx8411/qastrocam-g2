#include "QCamClient.moc"
#include "QCam.hpp"
#include "qpushbutton.h"
#include "qtooltip.h"
#include "QCamUtilities.hpp"

QCamClient::QCamClient(): cam_(NULL) {
   paused_=true;
   cam_=NULL;
}

QCamClient::QCamClient(QCam & theCam) {
   paused_=true;
   cam_=NULL;
   connectCam(theCam);
}

void QCamClient::connectCam(QCam & theCam) {
   if (cam_) {
      disconnectCam();
   }
   cam_=&theCam;
   camConnected();
   connect(cam_,SIGNAL(newFrame()),this,SLOT(newFrame()));
   paused_=false;
}

void QCamClient::disconnectCam() {
   if (!cam_) {
      return;
   }
   disconnect(cam_,SIGNAL(newFrame()),this,SLOT(newFrame()));
   paused_=true;
   camDisconnected();
   cam_=NULL;
}

void QCamClient::disable(bool notactive) {
   if (!notactive) resume();
   else pause();
}

void QCamClient::pause() {
   if (!paused_) {
      paused_=true;
      disconnect(cam_,SIGNAL(newFrame()),this,SLOT(newFrame()));
   }
}
void QCamClient::resume() {
   if (paused_) {
      paused_=false;
      connect(cam_,SIGNAL(newFrame()),this,SLOT(newFrame()));
   }
}

QWidget * QCamClient::buildGUI(QWidget *parent) {
   QPushButton * pauseCapture_=new QPushButton("pause",parent);
   QToolTip::add(pauseCapture_,
                 "Suspend the current capture");
   pauseCapture_->setToggleButton(true);
   pauseCapture_->setPixmap(*QCamUtilities::getIcon("movie_pause.png"));
   connect(pauseCapture_,SIGNAL(toggled(bool)),this,SLOT(disable(bool)));
   return parent;
}
