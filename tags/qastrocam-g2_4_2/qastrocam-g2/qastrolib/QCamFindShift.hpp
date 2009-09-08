#ifndef _QCamFindShift_hpp_
#define _QCamFindShift_hpp_

#include "QTelescope.hpp"
#include "QCamClient.hpp"
#include "QCamFrame.hpp"
#include "ShiftInfo.hpp"


/** Base class for classes used to tind the shift between succesives
    frames.
*/
class QCamFindShift : public QCamClient {
   Q_OBJECT;
public:
   QCamFindShift();
   QCamFindShift(QTelescope * scope);
   virtual QCamFrame image() const;
   ShiftInfo currentShift() const { return currentShift_;}
   virtual QWidget * buildGUI(QWidget *parent);
public slots:
   /** forget current reference */ 
   void reset();
protected:
   void camConnected();
   void camDisconnected();
   /** to register the first frame which
       will be the reference.
       return true if the registration is
       succesfull. Will be called until one
       call is successful.*/
   virtual bool registerFirstFrame()=0;
   /** return the shift beetween the current frame
       and the one used as reference by registerFirstFrame().
       if shift not found, return false. */
   virtual bool findShift(ShiftInfo & shift)=0;
protected slots:
   void newFrame();
signals:
   void shift(const ShiftInfo & );
private:
   bool firstFrameRegistered_;
   ShiftInfo currentShift_;
   /** will be used for an automatic reset
       if the frame size from the QCam changes.*/
   QSize currentFrameSize_;
   QTelescope* scope_;
};
#endif
