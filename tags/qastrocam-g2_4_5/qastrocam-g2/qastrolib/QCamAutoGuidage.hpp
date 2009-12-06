#ifndef _QCamAutoGuidage_hpp_
#define _QCamAutoGuidage_hpp_

#include <qobject.h>

class QCam;
class QCamFindShift;
class QTelescope;
class QWidget;
class ShiftInfo;

enum MoveDir {
   NotMoved,
   MovedEast,
   MovedWest,
   MovedNorth,
   MovedSouth
};


/** base class of any object who want to control
    a telescope to do automatic tracking of object.
    The only thing to implement is the slot frameShift(const QPoint& shift).
*/
class QCamAutoGuidage : public QObject {
   Q_OBJECT;
public:
   QCamAutoGuidage();
   /** the the camera.*/
   void setCam(QCam * cam);
   /** set the telscope. */
   void setScope(QTelescope * scope);
   /** set the FindShift algorithm.*/
   void setTracker(QCamFindShift * tracker);
   /** build the associated GUI */
   virtual QWidget * buildGUI(QWidget *parent=0);
   const QCam * cam() const { return cam_;}
signals:
   /** is emited when alt move is done */
   void altMove(MoveDir);
   /** is emited when asc move is done */
   void ascMove(MoveDir);
public slots:
  /** To activate or desactivate the Tracking.
      tracking is possible only if a cam, a telescope and a findShift
      algorithm have been connected.
   */
   void track(bool mode);
protected slots:
   /** called with the shift frome the original frame
       to the current frame.
       Must be implemented by any subclass.
   */
   virtual void frameShift(const ShiftInfo& shift)=0;
protected:
   QTelescope * telescope_;
   MoveDir lastAltMove_;
   MoveDir lastAscMove_;
   void moveAsc(MoveDir EWmove);
   void moveAlt(MoveDir NSmove);

private:
   QCam * cam_;
   QCamFindShift * tracker_;
   bool isTracking_;
};
#endif
