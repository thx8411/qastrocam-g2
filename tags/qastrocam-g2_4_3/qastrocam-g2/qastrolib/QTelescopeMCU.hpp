#ifndef _QTelescopeMCU_hpp_
#define _QTelescopeMCU_hpp_

#include "QTelescope.hpp"
#include <string>

using namespace std;

/** controle a Meade telescope via the serial port of
    the MCU controler.
    At this time there is a bug: it is possible to send
    data to the MCU, but not to receive.
    Original code from Gernot Stenz */

class QTelescopeMCU : public QTelescope {
   Q_OBJECT;
public:
   QTelescopeMCU(const char * deviceName);
   void buildGUI(QWidget * parent);
public slots:
   virtual void goE(float s=0) {sendCommand(moveEast);};
   virtual void goW(float s=0) {sendCommand(moveWest);};
   virtual void goS(float s=0) {sendCommand(moveSouth);};
   virtual void goN(float s=0) {sendCommand(moveNorth);};
   virtual void stopE() {sendCommand(stopMoveEast);};
   virtual void stopN() {sendCommand(stopMoveNorth);};
   virtual void stopW() {sendCommand(stopMoveWest);};
   virtual void stopS() {sendCommand(stopMoveSouth);};
   virtual double setSpeed(double speed);
   virtual void setSpeed(char speed);
   virtual bool setTracking(bool activated);
protected:
   /** send a command to the MCU, wait for the result and
       return it. */
   enum CommandType {
      park,
      getAlignment,
      setAlignment,
      moveSouth,
      moveNorth,
      moveEast,
      moveWest,
      stopMoveEast,
      stopMoveWest,
      stopMoveNorth,
      stopMoveSouth,
      setMoveSpeed
   };
   enum TrackingMode {
      land,
      polar,
      altAz,
      german
   };
   enum SubVersion {
      versionDate,
      versionFull,
      versionNumber,
      versionTime,
      productName
   };
   enum ReturnType {
      singleChar,
      booleans,
      numerics,
      strings,
      revision,
      none
   };
   string sendCommand(CommandType com,const string & param="");
   void setTracking(TrackingMode);
   string version(SubVersion v);
private:
   /** really send the command. */
   bool sendCmd(const string & cmd,const string & param="");
   /** read the result. */
   string recvCmd(ReturnType t);
   /// the file decritor of the serial port.
   int descriptor_;
   TrackingMode aligment_;
};
#endif
