#ifndef _QTelescopeAPM_hpp_
#define _QTelescopeAPM_hpp_

#include "QTelescope.hpp"

#include "PPort.hpp"

/** Telescope implementation for APM interface. **/

class QTelescopeAPM : public QTelescope {
   Q_OBJECT;
public:
   QTelescopeAPM(const char * pport);
   ~QTelescopeAPM();
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
   bool go;
   bool stop;
   int portEntry;
   const char* portName;
   PPort* paralPort;
   enum BitControl { EastBit=4,WestBit=5,NorthBit=6,SouthBit=7};
};

#endif
