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


#include "Vector2D.hpp"
#include <iostream>

Vector2D::Vector2D() {
   x_=y_=0.0;
}

Vector2D::Vector2D(const Vector2D & other) {
   *this=other;
}

Vector2D::Vector2D(double x, double y) {
   set(x,y);
}

void Vector2D::set(double x,double y) {
   setX(x);
   setY(y);
}

void Vector2D::setX(double x) {
   x_=x;
}

void Vector2D::setY(double y) {
   y_=y;
}

Vector2D & Vector2D::operator=(const Vector2D & other) {
   set(other.x(),other.y());
   return *this;
}

Vector2D & Vector2D::operator*=(double val) {
   x_*=val;
   y_*=val;
   return *this;
}

Vector2D & Vector2D::operator/=(double val)  {
   x_/=val;
   y_/=val;
   return *this;
}

Vector2D & Vector2D::operator+=(const Vector2D&other )  {
   x_+=other.x();
   y_+=other.y();
   return *this;
}

Vector2D & Vector2D::operator-=(const Vector2D&other)  {
   x_-=other.x();
   y_-=other.y();
   return *this;
}

Vector2D operator*(const Vector2D & first, double val) {
   Vector2D tmp(first);
   tmp*=val;
   return tmp;
}

Vector2D operator/(const Vector2D & first ,double val)  {
   Vector2D tmp(first);
   tmp/=val;
   return tmp;
}

Vector2D operator+(const Vector2D & first,const Vector2D & second) {
   Vector2D tmp(first);
   tmp+=second;
   return tmp;
}

Vector2D operator-(const Vector2D & first,const Vector2D & second) {
   Vector2D tmp(first);
   tmp-=second;
   return tmp;
}

ostream & operator<<(ostream & str, const Vector2D &vect) {
   str<<"("<<vect.x()<<","<<vect.y()<<")";
   return str;
}
