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
