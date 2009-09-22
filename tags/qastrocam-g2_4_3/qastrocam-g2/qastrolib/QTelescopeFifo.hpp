#ifndef _QTelescopeFifo_hpp_
#define _QTelescopeFifo_hpp_

#include "QTelescope.hpp"

/** Telescope implementation for a generic fifo interface.
    
 */
class QTelescopeFifo : public QTelescope {
   Q_OBJECT;
public:
   QTelescopeFifo(const char * fifoPath);
   public slots:
   virtual void goE(float shift) {}
   virtual void goW(float shift) {}
   virtual void goS(float shift) {}
   virtual void goN(float shift) {}
   virtual void stopW() {}
   virtual void stopE() {}
   virtual void stopN() {} 
   virtual void stopS() {}
   virtual double setSpeed(double speed) {return speed; }
   virtual bool setTracking(bool activated) {return true; }
  private:
   int descriptor_;
};
#endif
