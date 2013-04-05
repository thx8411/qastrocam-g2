/*******************************************************************
Qastrocam-g2
Copyright (C) 2013 Blaise-Florentin Collin

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


#ifndef _QCamMoviAviLossless_libav_hpp_
#define _QCamMoviAviLossless_libav_hpp_


// only available if have libavifile
#if HAVE_LIBAV_H

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

#include "QCamMovie.hpp"

// lossless class for avi recording
class QCamMovieAviLossless : public QCamMovie {
   public:
      QCamMovieAviLossless();
      ~QCamMovieAviLossless();
      QWidget * buildGUI(QWidget  * father);
      // open stream
      bool openImpl(const string & seqName, const QCam & cam);
      // close stream
      void closeImpl();
      // add a frame
      bool addImpl(const QCamFrame & newFrame, const QCam & cam);
   private:
      AVOutputFormat* output_format;
      AVFormatContext* output_format_cx;
      AVCodec* output_codec;
      AVCodecContext* output_codec_cx;
      AVStream* output_video_stream;
      AVFrame* picture;
      uint8_t* video_outbuf;
      int video_outbuf_size;
};

#endif /* HAVE_LIBAV_H */

#endif
