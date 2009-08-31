#include "QCamFindShift.moc"
#include <iostream>

#include "Vector2D.hpp"
#include "ShiftInfo.hpp"
#include "QCam.hpp"

#include <qlabel.h>

QCamFindShift::QCamFindShift(){
   scope_=NULL;
}

QCamFindShift::QCamFindShift(QTelescope* scope){
   scope_=scope;
}

void QCamFindShift::reset() {

   cout << "reset tracker"<<endl;
   firstFrameRegistered_=false;
   if (scope_!=NULL) scope_->Reset();
}

void QCamFindShift::camDisconnected() {
}

void QCamFindShift::camConnected() {
   firstFrameRegistered_=false;
   currentFrameSize_=cam().size();
}

void QCamFindShift::newFrame() {
   if (currentFrameSize_!=cam().size()) {
      currentFrameSize_=cam().size();
      // frame size changed, reset tracker
      reset();
   }
   if (firstFrameRegistered_) {
      ShiftInfo theShift;
      if (findShift(theShift)) {
         currentShift_=theShift;
         //cout << "center = "<<theShift.center().x()<<","<<theShift.center().y()<<endl;
         //cout << "shift = "<<theShift.shift().x()<<","<<theShift.shift().y()<<endl;
         //cout << "rotation = "<<theShift.angle()<<endl;
         emit(shift(theShift));
      }
   } else {
      firstFrameRegistered_=registerFirstFrame();
   }
   if(scope_!=NULL) scope_->Update();
}

QCamFrame QCamFindShift::image() const {
   QCamFrame img;
   if (firstFrameRegistered_) {
      img.setSize(QSize(32,32));
      //img.clear();
      cout << "copying center "
           <<(int)currentShift().center().x()-15 <<"x"
           <<(int)currentShift().center().y()-15 << " , "
           <<(int)currentShift().center().x()+16 <<"x"
           <<(int)currentShift().center().y()+16 <<endl;
      
      img.copy(cam().yuvFrame(),
               (int)currentShift().center().x()-15,
               (int)currentShift().center().y()-15,
               (int)currentShift().center().x()+16,
               (int)currentShift().center().y()+16,
               0,0);
   }
   return img;
}

QWidget * QCamFindShift::buildGUI(QWidget *parent) {
   return QCamClient::buildGUI(parent);
   //return new QLabel("No controls",parent);
}
