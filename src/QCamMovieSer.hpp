/******************************************************************
Qastrocam-g2
Copyright (C) 2010-2013 Blaise-Florentin Collin

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


#ifndef _QCamMoviSer_hpp_
#define _QCamMoviSer_hpp_


#include <stdint.h>

#include "QCamMovie.hpp"

typedef struct {
   char FileID[14];
   uint32_t LuID;
   uint32_t ColorID;
   uint32_t LittleEndian;
   uint32_t ImageWidth;
   uint32_t ImageHeight;
   uint32_t PixelDepth;
   uint32_t FrameCount;
   char Observer[40];
   char Instrume[40];
   char Telescope[40];
   uint64_t DateTime;
   uint64_t DateTime_UTC;
} __attribute((packed)) serHeader;

// ser recording class
class QCamMovieSer : public QCamMovie {
public:
   QCamMovieSer();
   ~QCamMovieSer();
   QWidget* buildGUI(QWidget* father);
   // open the stream
   bool openImpl(const string & seqName, const QCam & cam);
   // close the stream
   void closeImpl();
   // add a frame
   bool addImpl(const QCamFrame & newFrame, const QCam & cam);
private:
   serHeader header;
   int fd;
   int frameNumber;
};

#endif

