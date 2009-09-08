#ifndef _QTelescopeAPM_hpp_
#define _QTelescopeAPM_hpp_

#include "QTelescope.hpp"

class PPort;
/** Telescope implementation for APM interface.
    see 
 */
class QTelescopeAPM : public QTelescope {
   Q_OBJECT;
public:
   QTelescopeAPM(PPort * pport);
   public slots:
   virtual void goE(float shift);
   virtual void goW(float shift);
   virtual void goS(float shift);
   virtual void goN(float shift);
   virtual void stopW();
   virtual void stopE();
   virtual void stopN();
   virtual void stopS();
   virtual double setSpeed(double speed);
   virtual bool setTracking(bool activated);
private:
   enum BitControl { EastBit=4,WestBit=5,NorthBit=6,SouthBit=7};
   PPort * paralPort;
};
#endif
