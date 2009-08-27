#include "QCamFindShift_barycentre.moc"
#include "QCam.hpp"
#include "QStreamTranslator.hpp"
#include "ShiftInfo.hpp"

QCamFindShift_barycentre::QCamFindShift_barycentre() {
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
