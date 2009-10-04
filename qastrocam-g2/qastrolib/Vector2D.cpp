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
