/******************************************************************
Qastrocam
Copyright (C) 2003-2009   Franck Sicard
Qastrocam-g2
Copyright (C) 2009-2013   Blaise-Florentin Collin

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

#include <math.h>

#include <Qt/qlabel.h>
#include <Qt/qtooltip.h>

#include "QCamVBox.hpp"
#include "QCamFindShift_hotSpot.hpp"
#include "QCam.hpp"
#include "QStreamTranslator.hpp"
#include "ShiftInfo.hpp"
#include "QCamSlider.hpp"
#include "QFrameDisplay.hpp"
#include "QCamComboBox.hpp"
#include "QTelescope.hpp"
#include "QCamUtilities.hpp"

QCamFindShift_hotSpot::QCamFindShift_hotSpot(){
   searchBoxSize_=30;
   seuil_=0;
   autoSeuil_=true;
   lastBrightness_=0;
   binning_=1;

   mainBox_=NULL;
   seuilSlider_=NULL;
   bigBoxSlider_=NULL;
   dispImgCenter_=NULL;
}

QCamFindShift_hotSpot::QCamFindShift_hotSpot(QTelescope* scope) : QCamFindShift(scope) {
   searchBoxSize_=30;
   seuil_=0;
   autoSeuil_=true;
   lastBrightness_=0;
   binning_=1;

   mainBox_=NULL;
   seuilSlider_=NULL;
   bigBoxSlider_=NULL;
   dispImgCenter_=NULL;
}

double QCamFindShift_hotSpot::computeBarycenter(const Vector2D & from,
                                                int seuil,int step,
                                                int size,
                                                Vector2D & bary,
                                                double maximalCoverage) const {
   const unsigned char * img=cam().yuvFrame().Y();
   double imgSum=0;
   int pixelFound=0;
   int shift=size/2;
   int minX=(int)(from.x()-shift),
       minY=(int)(from.y()-shift),
       maxX=(int)(from.x()+shift),
       maxY=(int)(from.y()+shift);
   if (minX<0) minX=0;
   else if (maxX>cam().size().width()) maxX=cam().size().width();
   if (minY<0) minY=0;
   else if (maxY>cam().size().height()) maxY=cam().size().height();

   bary.set(0,0);
   for(int j=minY;
       j<maxY;j+=step) {
      for (int i=minX;
           i<maxX;i+=step) {
         int val=computePixelWeight(img[i+j*cam().size().width()]);
         if (val <=0) continue;
         imgSum += val;
         ++pixelFound;
         bary+=Vector2D(i,j)*val;
      }
   }

   bary/=imgSum;

   if (pixelFound > ((size/step)*(size/step)*maximalCoverage)) {
      imgSum*=-1;
   }

   return imgSum;
}

int QCamFindShift_hotSpot::computeSeuil() const {
   const unsigned char * img=cam().yuvFrame().Y();
   int max=0, average=0;
   for(int j=cam().size().height()-1;
       j>=0;--j) {
      for (int i=cam().size().width()-1;
           i>=0;--i) {
         unsigned char val=img[i+j*cam().size().width()];
         average+=val;
         if (val>max) max=val;
      }
   }
   int maxCoeef=3;
   int averageCoef=1;
   return ((average/cam().size().height()/cam().size().width())*averageCoef
           +max*maxCoeef)/(maxCoeef+averageCoef);
}

bool QCamFindShift_hotSpot::findHotSpot(Vector2D & center) {
   center.set(0,0);

   Vector2D newHotSpot;
   double newBrightness, hotSpotBrightness=0;

   for(int j=cam().size().height()-1-searchBoxSize_/2;j>=0;j-=searchBoxSize_/2) {
      for (int i=cam().size().width()-1-searchBoxSize_/2;i>=0;i-=searchBoxSize_/2) {
         newBrightness=findHotSpot(Vector2D(i,j),newHotSpot);
         if (newBrightness>hotSpotBrightness) {
            hotSpotBrightness=newBrightness;
            center=newHotSpot;
         }
      }
   }

   if (hotSpotBrightness>0) {
      //cerr << "hotspot = " << center <<endl;
      lastCenter_=center;
      lastBrightness_=hotSpotBrightness;
      return true;
   } else {
      //cerr << " hotspot not found"<<endl;
      return false;
   }
}

double QCamFindShift_hotSpot::findHotSpot(const Vector2D & from,
                                          Vector2D & center) {
   center.set(0,0);

   double newBrightness=0;

   /* first pass to find an approximation */
   newBrightness=computeBarycenter(from,seuil_,2*binning_,
                                   searchBoxSize_,
                                   center,0.5);

   /* second pass for good value */
   newBrightness=computeBarycenter(center,seuil_,binning_,
                                   searchBoxSize_*2/3,
                                   center,0.5*3/2);
   return newBrightness;
}

bool QCamFindShift_hotSpot::registerFirstFrame() {
   if (autoSeuil_) {
      seuil_=computeSeuil();
   }
   return findHotSpot(firstHotSpot_);
}

bool QCamFindShift_hotSpot::findShift(ShiftInfo & shift) {
   Vector2D newCenter;

   double newBrightness = findHotSpot(lastCenter_,newCenter);
   if (fabs((newBrightness/lastBrightness_)-1)<0.2) {
      shift.setCenter(newCenter);
      shift.setShift(newCenter-firstHotSpot_);
      shift.setAngle(0);
      lastBrightness_=lastBrightness_;
      lastCenter_=newCenter;
      computeCenterImg(searchBoxSize_,newCenter);
      emit(searchBoxSizeChanged(searchBoxSize_));
      emit(seuilChanged(seuil_));
      if (dispImgCenter_) dispImgCenter_->frame(image());

      return true;
   } else {
      // fast search failed
      if (findHotSpot(newCenter)) {
         shift.setCenter(newCenter);
         shift.setShift(newCenter-firstHotSpot_);
         shift.setAngle(0);
         computeCenterImg(searchBoxSize_,newCenter);
         emit(searchBoxSizeChanged(searchBoxSize_));
         emit(seuilChanged(seuil_));
         if (dispImgCenter_) dispImgCenter_->frame(image());
         return true;
      } else {
         centerImg_.clear();
         emit(searchBoxSizeChanged(searchBoxSize_));
         emit(seuilChanged(seuil_));
         if (dispImgCenter_) dispImgCenter_->frame(image());
         return false;
      }
   }
}

QWidget* QCamFindShift_hotSpot::buildGUI(QWidget *parent) {
   mainBox_= new QCamVGroupBox("Hot Spot",parent);

   QCamUtilities::registerWidget(mainBox_);

   QCamUtilities::setQastrocamIcon(mainBox_);
   QCamHBox* hbox=new QCamHBox(mainBox_);
   dispImgCenter_ = new QFrameDisplay(hbox,"center");
   QCamVBox* vbox = new QCamVBox (hbox);
   seuilSlider_=new QCamSlider("Tresh",false,vbox,0,255,false,false);
   connect(seuilSlider_,SIGNAL(valueChange(int)),this,SLOT(setSeuil(int)));
   connect(this,SIGNAL(seuilChanged(int)),seuilSlider_,SLOT(setValue(int)));

   QCamHBox* hbox2 = new QCamHBox(vbox);
   QCheckBox* autoSeuil = new QCheckBox(tr("auto-tresh"),hbox2);
   connect(autoSeuil,SIGNAL(toggled(bool)),this,SLOT(setAutoSeuil(bool)));
   connect(this,SIGNAL(autoSeuilChanged(bool)),autoSeuil,SLOT(setChecked(bool)));
   autoSeuil->setToolTip(tr("automaticaly calculate 'optimal' tresh"));
   emit(autoSeuilChanged(autoSeuil_));

   QLabel* binningLabel=new QLabel("Binning:",hbox2);
   int binningValues[5]={1,2,3,4,5};
   const char* binningLabels[5]={"1x1","2x2","3x3","4x4","5x5"};
   QCamComboBox* binning =new QCamComboBox("Binning",hbox2,4,binningValues,binningLabels);
   connect(binning,SIGNAL(change(int)),this,SLOT(setBinning(int)));
   binning->setToolTip(tr("seting a high binning will speedup\nthe processing with big box size"));

   bigBoxSlider_=new QCamSlider("Boxsize",false,vbox,12,200,false,false);
   connect(bigBoxSlider_,SIGNAL(valueChange(int)),this,SLOT(setSearchBoxSize(int)));
   connect(this,SIGNAL(searchBoxSizeChanged(int)),bigBoxSlider_,SLOT(setValue(int)));

   QCamFindShift::buildGUI(mainBox_);

   return mainBox_;
}

void QCamFindShift_hotSpot::setSeuil(int value) {
   seuil_=value;
   autoSeuil_=false;
   emit(seuilChanged(seuil_));
   emit(autoSeuilChanged(false));
}

void QCamFindShift_hotSpot::setAutoSeuil(bool value) {
   if (value && !autoSeuil_) {
      reset();
   }
   autoSeuil_=value;
}

void QCamFindShift_hotSpot::setBinning(int value) {
   binning_=value;
   reset();
}

void QCamFindShift_hotSpot::setSearchBoxSize(int value) {
   searchBoxSize_=value;
   emit(searchBoxSizeChanged(value));
}

QCamFrame QCamFindShift_hotSpot::image() const {
   return centerImg_;
   return QCamFindShift::image();
}

void QCamFindShift_hotSpot::computeCenterImg(int size,const Vector2D & center) {
   unsigned char* lineY;
   unsigned char* lineU;
   unsigned char* lineV;

   if (size%4)
      size+=(4-size%4);

   centerImg_.setSize(QSize(size,size));
   centerImg_.clear();
   centerImg_.setMode(cam().yuvFrame().getMode());

   int minX=(int)center.x()-size/2;
   int minY=(int)center.y()-size/2;
   centerImg_.copy(cam().yuvFrame(),minX,minY,minX+size-1,minY+size-1,0,0);
   int low=(size/6);
   int high=size-(size/6);
   for (int j=low;j<high;j+=binning_) {
      lineY = centerImg_.YLineForUpdate(j);
      if(centerImg_.getMode()!=GreyFrame) {
         lineU = centerImg_.ULineForUpdate(j);
         lineV = centerImg_.VLineForUpdate(j);
      }
      for(int i=low;i<high;i+=binning_) {
         int val=computePixelWeight(lineY[i]);
         if (val>=0) {
            lineY[i]=val/255;
            if(centerImg_.getMode()!=GreyFrame) {
               lineU[i]=128;
               lineV[i]=128;
            }
         }
      }
   }
}

int QCamFindShift_hotSpot::computePixelWeight(int pixelVal) const {
   pixelVal-=seuil_;
   if(pixelVal>0) {
      pixelVal=pixelVal*255/(255-seuil_);
      pixelVal*=pixelVal;
   }
   return pixelVal;
}
