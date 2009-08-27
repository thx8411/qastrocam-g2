#ifndef __QKingClient_hpp_
#define __QKingClient_hpp_

#include "QCamFindShift_hotSpot.hpp"
#include <time.h>

class QStatusBar;

class QKingClient : public QCamFindShift_hotSpot {
   Q_OBJECT;
public:
   QKingClient();
   QWidget * buildGUI(QWidget *parent);
protected:
   bool registerFirstFrame();
   bool findShift(ShiftInfo & shift);
private:
   /** time in second of the firts frame */
   time_t timeFirstFrame_;
   QStatusBar * statusBar_;
};

#endif
