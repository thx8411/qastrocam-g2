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
   // update shitfing value when a new frame comes
   virtual void Update(double x, double y);
   // creat new files when the tracker is reset
   virtual void Reset();
   // parent class compatibility
   virtual void setTrack(bool tracking);
public slots:
   // moves
   virtual void goE(float shift){};
   virtual void goW(float shift){};
   virtual void goS(float shift){};
   virtual void goN(float shift){};
   // stop
   virtual void stopW() {}
   virtual void stopE() {}
   virtual void stopN() {}
   virtual void stopS() {}
   // speed and track
   virtual double setSpeed(double speed) {return speed; }
   virtual bool setTracking(bool activated) {return true; }
  private:
   // timing
   double getTime();
   double sessionTime;
   void writeToFile();
   // files
   string genericFilePath;
   string filePathx;
   string filePathy;
   FILE* descriptorx_;
   FILE* descriptory_;
   // current datas
   float xPosition;
   float yPosition;
   bool tracking_;
};

#endif
