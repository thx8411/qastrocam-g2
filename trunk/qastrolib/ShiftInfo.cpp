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
