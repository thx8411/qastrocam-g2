#ifndef _QCamFindShift_barycentre_hpp_
#define _QCamFindShift_barycentre_hpp_

#include "QCamFindShift.hpp"
#include "Vector2D.hpp"

/** Find the shift between two frame by computing the barycenter
    of the frame. */
class QCamFindShift_barycentre : public QCamFindShift {
   Q_OBJECT;
public:
   QCamFindShift_barycentre();
protected:
   bool registerFirstFrame();
   bool findShift(ShiftInfo & shift);
private:
   bool findBarycentre(Vector2D & shift);
   Vector2D lastBarycentre_;
   double average_;
   int maximum_;
};

#endif
