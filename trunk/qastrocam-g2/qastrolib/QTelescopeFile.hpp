#ifndef _QTelescopeFile_hpp_
#define _QTelescopeFile_hpp_

#include <string>

#include "QTelescope.hpp"

using namespace std;

/** Telescope implementation for a file interface.
    
 */
class QTelescopeFile : public QTelescope {
   Q_OBJECT;
public:
   QTelescopeFile(const char * filePath);
   virtual void Update();
   virtual void setTrack(bool tracking);
   public slots:
   virtual void goE(float shift);
   virtual void goW(float shift);
   virtual void goS(float shift);
   virtual void goN(float shift);
   virtual void stopW() {}
   virtual void stopE() {}
   virtual void stopN() {} 
   virtual void stopS() {}
   virtual double setSpeed(double speed) {return speed; }
   virtual bool setTracking(bool activated) {return true; }
  private:
   double getTime();
   double sessionTime;
   void writeToFile();
   string filePathx;
   string filePathy;
   FILE* descriptorx_;
   FILE* descriptory_;
   float xPosition;
   float yPosition;
   bool tracking_;
};
#endif
