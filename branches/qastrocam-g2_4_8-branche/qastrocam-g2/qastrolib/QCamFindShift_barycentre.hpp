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


#ifndef _QCamFindShift_barycentre_hpp_
#define _QCamFindShift_barycentre_hpp_

#include "QCamFindShift.hpp"
#include "Vector2D.hpp"
#include "QTelescope.hpp"

/** Find the shift between two frame by computing the barycenter
    of the frame. */
class QCamFindShift_barycentre : public QCamFindShift {
   Q_OBJECT;
public:
   QCamFindShift_barycentre();
   QCamFindShift_barycentre(QTelescope* scope);
protected:
   bool registerFirstFrame();
   bool findShift(ShiftInfo & shift);
private:
   bool findBarycentre(Vector2D & shift);
   Vector2D lastBarycentre_;
   double average_;
   int maximum_;
};

#endif
