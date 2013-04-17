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


#ifndef _FITSIMAGE_HPP_
#define _FITSIMAGE_HPP_

#if HAVE_CFITSIO_H

#define IMAGE_WIDTH      640
#define IMAGE_HEIGHT     480

#define WORD short

#include <string>
#include <map>

using namespace std;

class QCamFrame;

class FitsImage {
protected:
   const string fileName_;
public:
   FitsImage(const string & fileName): fileName_(fileName) {}
   virtual bool load(QCamFrame &)=0;
   virtual bool save(const QCamFrame &)=0;
   virtual ~FitsImage() {}
   virtual bool close() {}
};

#include <fitsio.h>

class FitsImageCFITSIO : public FitsImage {
   fitsfile *fptr_;
   bool multipleFrame_;
   int status_;
   int numAxes_;
   long axesDim_[3];
   long base_[3];
private:
   bool initFits(const QCamFrame& frame);
public:
   FitsImageCFITSIO(const string & fileName,bool multipleFrame=false);
   ~FitsImageCFITSIO();
   bool load(QCamFrame &);
   bool save(const QCamFrame &);
   virtual bool close();
};

#endif /* HAVE_CFITSIO_H */

#endif
