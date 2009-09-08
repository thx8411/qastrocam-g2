#ifndef _SCmod_hpp_
#define _SCmod_hpp_
/** Abstract class to control an SC moded webcam
 */
class QWidget;

class SCmod {
public:
   virtual void enterLongPoseMode()=0;
   virtual void leaveLongPoseMode()=0;
   virtual void stopAccumulation()=0;
   virtual void startAccumulation()=0;
   virtual ~SCmod() {}
   virtual QWidget * buildGUI(QWidget * parent) { return parent; }
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
   SCmodSerialPort(const char * device);
   void enterLongPoseMode();
   void leaveLongPoseMode();
   void stopAccumulation();
   void startAccumulation();
private:
   int device_;
};

#endif
