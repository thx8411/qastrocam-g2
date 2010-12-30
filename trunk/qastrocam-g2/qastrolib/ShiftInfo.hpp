/******************************************************************
Qastrocam
Copyright (C) 2003-2009   Franck Sicard
Qastrocam-g2
Copyright (C) 2009   Blaise-Florentin Collin

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License v2
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
MA  02110-1301, USA.
*******************************************************************/


#ifndef _ShiftInfo_hpp_
#define _ShiftInfo_hpp_

#include "Vector2D.hpp"

/** this class is used to describe the shift between 2 frames.
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
