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


#include "QCamFindShift_barycentre.hpp"
#include "QCam.hpp"
#include "QStreamTranslator.hpp"
#include "ShiftInfo.hpp"
#include "QTelescope.hpp"

QCamFindShift_barycentre::QCamFindShift_barycentre() {
   average_=0;
}

QCamFindShift_barycentre::QCamFindShift_barycentre(QTelescope* scope) : QCamFindShift(scope) {
   average_=0;
}

bool QCamFindShift_barycentre::findBarycentre(Vector2D & center) {
   const unsigned char * img=cam().yuvFrame().Y();
   double imgSum=0, barySum=0;
   int max=0;
   center.set(0,0);
   int seuil=(maximum_+(int)average_)>>1;
   for(int j=cam().size().height()-1;
       j>=0;--j) {
      for (int i=cam().size().width()-1;
           i>=0;--i) {
         unsigned char val=img[i+j*cam().size().width()];
         imgSum += val;
         if (val>max) max=val;
         if (val>=seuil) {
            barySum+=val;
            center+=Vector2D(i,j)*val;
         }
      }
   }
   average_=imgSum/(cam().size().height()*cam().size().width());
   maximum_=max;
   if (barySum>0) {
      center/=barySum;
      return true;
   } else {
      return false;
   }
}

bool QCamFindShift_barycentre::registerFirstFrame() {
   average_=0;
   maximum_=0;
   int i=cam().size().height()*cam().size().width();
   const unsigned char * img=cam().yuvFrame().Y();
   while (i-->0) {
      unsigned char val=img[i];
      if (val>maximum_) maximum_=val;
      average_+=val;
   }
   average_/=cam().size().height()*cam().size().width();
   return findBarycentre(lastBarycentre_);
}

bool QCamFindShift_barycentre::findShift(ShiftInfo & shift) {
   Vector2D newCenter;
   if (findBarycentre(newCenter)) {
      cout << "barycentre = " << newCenter <<endl;
      shift.setCenter(newCenter);
      shift.setShift(newCenter-lastBarycentre_);
      shift.setAngle(0);
      return true;
   } else {
      return false;
   }
}
