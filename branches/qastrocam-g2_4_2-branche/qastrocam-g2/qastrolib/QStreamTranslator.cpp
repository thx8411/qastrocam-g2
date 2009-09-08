#include "QStreamTranslator.hpp"

#include <iostream>
#include <qpoint.h>

ostream & operator<<(ostream & str,const QPoint & point) {
   str <<'('<<point.x()<<','<<point.y()<<')';
   return str;
}
