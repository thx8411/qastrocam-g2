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


#ifndef _QCamFindShift_hotSpot_hpp_
#define _QCamFindShift_hotSpot_hpp_

#include "QCamFindShift.hpp"
#include "Vector2D.hpp"
#include "QTelescope.hpp"

class Q3VGroupBox;
class QCamSlider;
class QFrameDisplay;

/** Find the shift between two frame by looking at the brighter hot spot
    of the frame. */
class QCamFindShift_hotSpot : public QCamFindShift {
   Q_OBJECT
public:
   QCamFindShift_hotSpot();
   QCamFindShift_hotSpot(QTelescope*);
   QWidget * buildGUI(QWidget *);
   virtual QCamFrame image() const;
public slots:
   /** only pixel above this value (tresh) are taken in account */
   void setSeuil(int value);
   /** activate the auto mode for the tresh calculation */
   void setAutoSeuil(bool value);

   /** size of the moving box used to search the hot spot.
       size must be smaller than the distance between two
       possible hot spot.
    */
   void setSearchBoxSize(int value);

   /** binning applyed in the search box when
       searching the hotspot. Will speed up search when
       box is big, butt will be less accurate */
   void setBinning(int value);
signals:
   void seuilChanged(int);
   void autoSeuilChanged(bool);
   void searchBoxSizeChanged(int);
protected:
   bool registerFirstFrame();
   bool findShift(ShiftInfo & shift);
   Vector2D firstHotSpot_;
private:
   void computeCenterImg(int size,const Vector2D & center);
   bool findHotSpot(Vector2D & shift);
   double findHotSpot(const Vector2D & from,
                      Vector2D & center);
   int computeSeuil() const;
   int computePixelWeight(int pixelVal) const {
      pixelVal-=seuil_;
      if (pixelVal>0) {
         pixelVal=pixelVal*255/(255-seuil_);
         pixelVal*=pixelVal;
      }
      return pixelVal;
   }
   double computeBarycenter(const Vector2D & from,
                            int seuil,int step,
                            int size,
                            Vector2D & bary,
                            double maxCoverage) const;
   double lastBrightness_;
   Vector2D lastCenter_;

   int searchBoxSize_;
   int seuil_;

   int binning_;

   QCamFrame centerImg_;

   /* GUI data */
   Q3VGroupBox * mainBox_;
   QCamSlider * seuilSlider_;
   QCamSlider * bigBoxSlider_;
   QFrameDisplay * dispImgCenter_;
   bool autoSeuil_;
};

#endif
