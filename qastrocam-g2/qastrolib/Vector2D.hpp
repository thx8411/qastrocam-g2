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


#ifndef _Vector2D_hpp_
#define _Vector2D_hpp_

/** simple vector class in 2 dimension.
    Like QPoint but with float.
*/

#include <ostream>

using namespace std;

class Vector2D {
public:
   Vector2D();
   Vector2D(const Vector2D&);
   Vector2D(double x, double y);
   Vector2D & operator=(const Vector2D &);
   void set(double x,double y);
   void setX(double x);
   void setY(double y);
   Vector2D & operator*=(double val);
   Vector2D & operator/=(double val);
   Vector2D & operator+=(const Vector2D&);
   Vector2D & operator-=(const Vector2D&);
   const double & x() const { return x_;}
   const double & y() const { return y_;}
private:
   double x_;
   double y_;
};

Vector2D operator*(const Vector2D&,double val);
Vector2D operator/(const Vector2D&,double val);
Vector2D operator+(const Vector2D&,const Vector2D &);
Vector2D operator-(const Vector2D&,const Vector2D &);

ostream & operator<<(ostream & , const Vector2D &);

#endif
