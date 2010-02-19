/******************************************************************
Qastrocam
Copyright (C) 2003-2009   Franck Sicard
Qastrocam-g2
Copyright (C) 2009-2010 Blaise-Florentin Collin

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


#ifndef _QCamMoviAvi_hpp_
#define _QCamMoviAvi_hpp_

#include "../config.h"

#if HAVE_AVIFILE_H

#include "QCamMovie.hpp"

#include <avifile.h>

// raw avi recording class
class QCamMovieAvi : public QCamMovie {
public:
   QCamMovieAvi();
   ~QCamMovieAvi();
   QWidget* buildGUI(QWidget* father);
   // open the stream
   bool openImpl(const string & seqName, const QCam & cam);
   // close the stream
   void closeImpl();
   // add a frame
   bool addImpl(const QCamFrame & newFrame, const QCam & cam);
private:
   // file
   mutable avm::IWriteFile* aviFile_;
   // stream
   mutable avm::IWriteStream* aviStream_;
   // frame buffer
   mutable unsigned char* deinterlaceBuf_;
};

#endif

#endif
