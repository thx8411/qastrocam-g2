#ifndef _SCmodParPortPPdev_hpp_
#define _SCmodParPortPPdev_hpp_

#include "SCmod.hpp"
#include "qobject.h"

class PPort;
class QCamComboBox;

class SCmodParPortPPdev : public QObject, public SCmod {
   Q_OBJECT
public:
   SCmodParPortPPdev();
   virtual ~SCmodParPortPPdev();
   void enterLongPoseMode();
   void leaveLongPoseMode();
   void stopAccumulation();
   void startAccumulation();
   QWidget * buildGUI(QWidget * parent);
public slots:
   void setPPort(int ioPort);
private:
   enum pportBit {evenLinesTransferOn=0,
                  oddLinesTransferOn=1,
                  preampOn=2,
                  shutterOn=3};
   void sendPportCmd();
   void activatePPort();
   int ppdev_fd;
   int data_out;
   QCamComboBox * ioPortSelect_;
};

#endif
