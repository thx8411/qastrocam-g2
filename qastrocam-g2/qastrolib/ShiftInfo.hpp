#ifndef _ShiftInfo_hpp_
#define _ShiftInfo_hpp_

#include "Vector2D.hpp"

/** this class is used to discribe the shift between 2 frames.
    it contain the translation to apply, followed by the
    rotation (in radian) to map the second frame to the reference one.
*/
class ShiftInfo {
public:
   ShiftInfo();
   ShiftInfo(const Vector2D & center,
             const Vector2D & shift, double angle);
   void setCenter(const Vector2D & vect);
   void setShift(const Vector2D & vect);
   void setAngle(double angle);
   /** return the position of the reference point.
    */
   const Vector2D & center() const;
   /** return the translation vector.
    */
   const Vector2D & shift() const;
   /** return the rotation angle (around the center).
    */
   const double angle() const;
private:
   Vector2D center_;
   Vector2D vectorShift_;
   double angleRotation_;
   
};
#endif
