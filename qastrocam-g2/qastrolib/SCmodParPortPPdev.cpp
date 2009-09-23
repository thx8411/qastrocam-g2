#include "SCmodParPortPPdev.moc"
#include "PPort.hpp"
#include <unistd.h>
#include <qwidget.h>
#include <iostream>
#include "QCamComboBox.hpp"
#include <qmessagebox.h>

/*****************************************************
 * rewrite of SCmodParPort.cpp
 * - use ppdev to access the pport
 *   -> no need for suid root
 * (works only for PC-style pports i think)
 *
 * >>>> make sure to load the ppdev kerneldriver
 *      (modprobe ppdev)
 *
 *                by simon '05  devel at auctionant.de
 *****************************************************/

#include <signal.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/parport.h>
#include <linux/ppdev.h>

#include "SettingsBackup.hpp"

extern settingsBackup settings;

SCmodParPortPPdev::~SCmodParPortPPdev() {
   if (ppdev_fd != -1) {
      close(ppdev_fd);
   }
}

SCmodParPortPPdev::SCmodParPortPPdev() {
   if(settings.haveKey("LX_DEVICE"))
      device=settings.getKey("LX_DEVICE");
   else
      device="/dev/parport0";
   ppdev_fd = open(device.c_str(), O_RDWR, 0);
   if (ppdev_fd == -1) {
      perror(device.c_str());
   }
   
   //claim ppdev:
   if (ioctl(ppdev_fd, PPCLAIM, 0) != 0) {
      perror("ioctl PPCLAIM");
      close(ppdev_fd);
      ppdev_fd = -1;
   }
   
   if (ppdev_fd != -1) {
      //set direction to OUTPUT on d0-d7:
      int outputmode=0;
      if (ioctl(ppdev_fd, PPDATADIR, &outputmode) != 0){
         perror("ioctl PPDATADIR");
      }
   }
   
   activatePPort();
}

void SCmodParPortPPdev::activatePPort() {
   data_out = 0x00;

   leaveLongPoseMode();
   stopAccumulation();
} 
   
void SCmodParPortPPdev::sendPportCmd() {
   printf(">> sending data=%d = %d%d%d%d (shutter,amp,odd,even)\n",data_out,
		   (data_out&(1<<shutterOn)?1:0),
		   (data_out&(1<<preampOn)?1:0),
		   (data_out&(1<<oddLinesTransferOn)?1:0),
		   (data_out&(1<<evenLinesTransferOn)?1:0)
		   );
   	
  if (ppdev_fd != -1)
   ioctl(ppdev_fd, PPWDATA, &data_out);
  else
   printf("ERROR: cant write data to ppdev (=-1) !\n");
}

void SCmodParPortPPdev::enterLongPoseMode() {
   data_out &= ~(1<<shutterOn);
}

void SCmodParPortPPdev::leaveLongPoseMode() {
   data_out |= (1<<shutterOn);
}

void SCmodParPortPPdev::stopAccumulation() {
   data_out |= (1<<preampOn);
   
   sendPportCmd();
   usleep(800);
   
   data_out |= (1<<evenLinesTransferOn);
   data_out |= (1<<oddLinesTransferOn);
   sendPportCmd();
   
}

void SCmodParPortPPdev::startAccumulation() {
   data_out &= ~(1<<evenLinesTransferOn);
   data_out &= ~(1<<oddLinesTransferOn);
   data_out &= ~(1<<preampOn);
   sendPportCmd();
}


/*QWidget * SCmodParPortPPdev::buildGUI(QWidget * parent) {
   int portTable[]={0,1,2};
   const char * portLabel[]={"/dev/parport0","/dev/parport1","/dev/parport2"};
   ioPortSelect_=new QCamComboBox("PPort",parent,3,portTable,portLabel);
   connect(ioPortSelect_,SIGNAL(change(int)),
           this,SLOT(setPPort(int)));
   ioPortSelect_->show();
   return parent;
}*/
