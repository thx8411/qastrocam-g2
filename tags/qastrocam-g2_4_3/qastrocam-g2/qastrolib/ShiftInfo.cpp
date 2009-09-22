#include "ShiftInfo.hpp"

ShiftInfo::ShiftInfo() :
   center_(0,0),
   vectorShift_(0,0),
   angleRotation_(0) {
}

ShiftInfo::ShiftInfo(const Vector2D & center,
                     const Vector2D & shift, double angle) :
   center_(center),
   vectorShift_(shift),
   angleRotation_(angle) {
}
   
void ShiftInfo::setShift(const Vector2D & vect) {
   vectorShift_=vect;
}

void ShiftInfo::setCenter(const Vector2D & vect) {
   center_=vect;
}

void ShiftInfo::setAngle(double angle) {
   angleRotation_=angle;
}
      
const Vector2D & ShiftInfo::shift() const {
   return vectorShift_;
}

const Vector2D & ShiftInfo::center() const {
   return center_;
}

const double ShiftInfo::angle() const {
   return angleRotation_;
}
