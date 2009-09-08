#ifndef _SCmodParPort_hpp_
#define _SCmodParPort_hpp_

#include "SCmod.hpp"
#include "qobject.h"

class PPort;
class QCamComboBox;

class SCmodParPort : public QObject, public SCmod {
   Q_OBJECT
public:
   SCmodParPort();
   virtual ~SCmodParPort();
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
   void setPPort(PPort * paralPort);
   PPort * pport_;
   QCamComboBox * ioPortSelect_;
};

#endif
