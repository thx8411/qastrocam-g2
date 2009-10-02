#ifndef _SCmod_hpp_
#define _SCmod_hpp_
/** Abstract class to control an SC moded webcam
 */

#include <string>

using namespace std;

class QWidget;

class SCmod {
public:
   virtual void enterLongPoseMode() {};
   virtual void leaveLongPoseMode() {};
   virtual void stopAccumulation() {};
   virtual void startAccumulation() {};
   virtual ~SCmod();
   virtual QWidget * buildGUI(QWidget * parent) { return parent; }
   void setLevels(bool polarity);
protected :
   bool inverted_;
};


class QCamVesta;
class SCmodTucLed : public SCmod {
public:
   SCmodTucLed(QCamVesta & cam);
   void enterLongPoseMode();
   void leaveLongPoseMode();
   void stopAccumulation();
   void startAccumulation();
private:
   QCamVesta & cam_;
};

class SCmodSerialPort : public SCmod {
public:
   SCmodSerialPort();
   void enterLongPoseMode();
   void leaveLongPoseMode();
   void stopAccumulation();
   void startAccumulation();
private:
   string device;
   int device_;
};

#endif
