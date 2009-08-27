#include "SCmodParPort.moc"
#include "PPort.hpp"
#include <unistd.h>
#include <qwidget.h>
#include <iostream>
#include "QCamComboBox.hpp"

SCmodParPort::SCmodParPort() {
   pport_=NULL;
   ioPortSelect_=NULL;
}

SCmodParPort::~SCmodParPort() {
   if (pport_) {
      delete pport_;
   }
   if (ioPortSelect_) {
      delete ioPortSelect_;
   }
}

void SCmodParPort::setPPort(int ioPort) {
   setPPort(PPort::getPPort(ioPort));
}

void SCmodParPort::setPPort(PPort * paralPort) {
   pport_=paralPort;
   //initRemoteControlLongExposure(remoteCTRL_);
   if (!paralPort->registerBit(this,evenLinesTransferOn)) {
      //remoteCTRLframeRateMultiplicateur_->hide();
      std::cerr << "cant' register bit "<<evenLinesTransferOn<<" on // port\n";
   }
   if (!paralPort->registerBit(this,oddLinesTransferOn)) {
      std::cerr << "cant' register bit "<< oddLinesTransferOn <<" on // port\n";
   }
   if (!paralPort->registerBit(this,preampOn)) {
      std::cerr << "cant' register bit "<<preampOn<<" on // port\n";
   }
   if (!paralPort->registerBit(this,shutterOn)) {
      std::cerr << "cant' register bit "<<shutterOn<<" on // port\n";
   }
   leaveLongPoseMode();
   stopAccumulation();
}

void SCmodParPort::sendPportCmd() {
   pport_->commit();
}

void SCmodParPort::enterLongPoseMode() {
   pport_->setBit(this,shutterOn,false);
}

void SCmodParPort::leaveLongPoseMode() {
   pport_->setBit(this,shutterOn,true);
}

void SCmodParPort::stopAccumulation() {
   pport_->setBit(this,preampOn,true);
   sendPportCmd();
   usleep(800);
   pport_->setBit(this,evenLinesTransferOn,true);
   pport_->setBit(this,oddLinesTransferOn,true);
   sendPportCmd();
}

void SCmodParPort::startAccumulation() {
   pport_->setBit(this,evenLinesTransferOn,false);
   pport_->setBit(this,oddLinesTransferOn,false);
   pport_->setBit(this,preampOn,false);
   sendPportCmd();
}


QWidget * SCmodParPort::buildGUI(QWidget * parent) {
   int portTable[]={0x378,0x278,0x3BC};
   const char * portLabel[]={"LPT1:0x378","LPT2:0x278","LPT3:0x3BC"};
   ioPortSelect_=new QCamComboBox("IO Port",parent,3,portTable,portLabel);
   connect(ioPortSelect_,SIGNAL(change(int)),
           this,SLOT(setPPort(int)));
   ioPortSelect_->show();
   return parent;
}
