/******************************************************************
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


// only available if have libav
#if HAVE_LIBAV_H

#include <stdlib.h>

#include "yuv.hpp"

#include "QCamMovieAvi_libav.hpp"
#include "QCamFrame.hpp"
#include "QCam.hpp"
#include "SettingsBackup.hpp"

// libav stores raw video datas upside down, with
// negative height in stream headers
// old gstream versions can't handle this negative height
// we change this height at offset 00b4 in the avi file
#define GSTREAM_HEIGHT_TWEAK

// gstream stream height tweak
#ifdef GSTREAM_HEIGHT_TWEAK
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#endif

// settings object, needed everywhere
extern settingsBackup settings;


/******************************/
/* QCamMovieAvi private stuff */
/******************************/

// getting the codec
void QCamMovieAvi::getCodec() {
   output_codec=avcodec_find_encoder(CODEC_ID_RAWVIDEO);
}

// getting pixel format
void QCamMovieAvi::getPixelformat(const QCam & cam) {
   if((cam.yuvFrame().getMode()==GreyFrame)&&!(registaxCompatibility))
      output_codec_cx->pix_fmt=PIX_FMT_GRAY8;
   else
      output_codec_cx->pix_fmt=PIX_FMT_RGB24;
}

// getting buffer size
void QCamMovieAvi::getBuffersize(const QCam & cam) {
   if ((cam.yuvFrame().getMode()==GreyFrame)&&!(registaxCompatibility))
      video_outbuf_size=cam.size().width()*cam.size().height();
   else
      video_outbuf_size=cam.size().width()*cam.size().height()*3;
}

// filling the frame
void QCamMovieAvi::fillFrame(const QCamFrame & newFrame) {
   // RGB24
   if(newFrame.getMode()!=GreyFrame) {
      yuv444_to_bgr24(newFrame.size().width(),newFrame.size().height(),newFrame.Y(),newFrame.U(),newFrame.V(),picture->data[0]);
   // 8 BITS GRAY
   } else {
      if(registaxCompatibility) {
         // force grey to RGB
         y_to_bgr24(newFrame.size().width(),newFrame.size().height(),newFrame.Y(),picture->data[0]);
      } else {
         memcpy(picture->data[0],newFrame.Y(),newFrame.size().height()*newFrame.size().width());
      }
   }
}

/**************************************/
/* QCamMovieAviLossless private stuff */
/**************************************/

// getting the codec
void QCamMovieAviLossless::getCodec() {
   output_codec=avcodec_find_encoder(CODEC_ID_HUFFYUV);
}

// getting pixel format
void QCamMovieAviLossless::getPixelformat(const QCam & cam) {
   if(cam.yuvFrame().getMode()==GreyFrame)
      output_codec_cx->pix_fmt=PIX_FMT_YUV422P;
   else
      output_codec_cx->pix_fmt=PIX_FMT_RGB32;
}

// getting buffer size
void QCamMovieAviLossless::getBuffersize(const QCam & cam) {
   /* worst case, RGB32 */
   video_outbuf_size=cam.size().height()*cam.size().width()*4;
}

// filling the frame
void QCamMovieAviLossless::fillFrame(const QCamFrame & newFrame) {
   // RGB32
   if(newFrame.getMode()!=GreyFrame) {
      yuv444_to_bgr32(newFrame.size().height(),newFrame.size().width(),newFrame.Y(),newFrame.U(),newFrame.V(),picture->data[0]);
   // 8 BITS (YUYV U=V=zero)
   } else {
      memcpy(picture->data[0],newFrame.Y(),newFrame.size().height()*newFrame.size().width());
      memset(picture->data[1],128,newFrame.size().height()*newFrame.size().width()/2);
      memset(picture->data[2],128,newFrame.size().height()*newFrame.size().width()/2);
   }
}

/**********************/
/* QCamMovieAvi class */
/**********************/

QCamMovieAvi::QCamMovieAvi() {
   // should the movie be registax compatible ?
   if(settings.haveKey("REGISTAX_AVI"))
      registaxCompatibility=(string(settings.getKey("REGISTAX_AVI"))=="yes");
   else
      registaxCompatibility=false;

   // init libav
   av_register_all();
}

QCamMovieAvi::~QCamMovieAvi() {
  // nothing to do
}

QWidget * QCamMovieAvi::buildGUI(QWidget  * father) {
   // nothing to do yet
   return father;
}

// open a stream
bool QCamMovieAvi::openImpl(const string & seqName, const QCam & cam) {
   uint8_t* picture_buffer;

   cam.writeProperties(seqName+".properties");

   // get output format
   output_format=av_guess_format("avi", NULL, NULL );
   if(!output_format) {
      fprintf(stderr,"Can't get AVI format, leaving...\n");
      exit(1);
   }

   // allocate context
   output_format_cx=avformat_alloc_context();
   if(!output_format_cx) {
      fprintf(stderr,"Can't allocate format context, leaving...\n");
      exit(1);
   }

   // format context setting
   output_format_cx->oformat=output_format;
   snprintf(output_format_cx->filename,sizeof(output_format_cx->filename),"%s",(seqName+".avi").c_str());

   // getting the codec
   getCodec();
   if(!output_codec) {
      fprintf(stderr,"Can't get codec, leaving...\n");
      exit(1);
   }

   // add the video stream
   output_video_stream=avformat_new_stream(output_format_cx,output_codec);
   if(!output_video_stream) {
      fprintf(stderr,"Can't add video stream, leaving...\n");
      exit(1);
   }

   // setting codec context
   output_codec_cx=output_video_stream->codec;
   output_codec_cx->width=cam.size().width();
   output_codec_cx->height=cam.size().height();
   output_codec_cx->time_base.den=atoi(cam.getProperty("FrameRateSecond").c_str());
   if(output_codec_cx->time_base.den<1) output_codec_cx->time_base.den=1;
   output_codec_cx->time_base.num=1;

   // setting codec pix format
   getPixelformat(cam);

   // opening codec
   if(avcodec_open2(output_codec_cx,output_codec,NULL)<0) {
      fprintf(stderr,"Can't open codec, leaving...\n");
      exit(1);
   }

   // buffer allocations
   getBuffersize(cam);
   video_outbuf=(uint8_t*)av_malloc(video_outbuf_size);
   if(!video_outbuf) {
      fprintf(stderr,"Can't allocate video buffer, leaving...\n");
      exit(1);
   }

   // picture allocation
   picture=avcodec_alloc_frame();
   if(!picture) {
      fprintf(stderr,"Can't allocate picture, leaving...\n");
      exit(1);
   }

   picture_buffer=(uint8_t*)av_malloc(avpicture_get_size(output_codec_cx->pix_fmt,cam.size().width(),cam.size().height()));
   if(!picture_buffer) {
      fprintf(stderr,"Can't allocate picture buffer, leaving...\n");
      exit(1);
   }
   avpicture_fill((AVPicture*)picture,picture_buffer,output_codec_cx->pix_fmt,cam.size().width(),cam.size().height());

   // dump format
   av_dump_format(output_format_cx, 0,(seqName+".avi").c_str(), 1);

   // opening file
   if(avio_open(&output_format_cx->pb,(seqName+".avi").c_str(),AVIO_FLAG_WRITE)<0) {
      fprintf(stderr,"Can't open file, leaving...\n");
      exit(1);
   }

   // write header
   avformat_write_header(output_format_cx,NULL);

   cam.writeProperties(seqName+".properties");
   return true;
}

// close a stream
void QCamMovieAvi::closeImpl() {
// gstream stream height tweak
#ifdef GSTREAM_HEIGHT_TWEAK
   int fd;
   int h;
   string name_;
   name_=output_format_cx->filename;
   h=output_codec_cx->height;
#endif

   // write trailer
   av_write_trailer(output_format_cx);
   // close codec
   avcodec_close(output_codec_cx);
   // free picture and buffer
   av_free(picture->data[0]);
   av_free(picture);
   av_free(video_outbuf);
   // free code and stream
   av_freep(&output_format_cx->streams[0]->codec);
   av_freep(&output_format_cx->streams[0]);
   // close file
   avio_close(output_format_cx->pb);
   // free format context
   av_free(output_format_cx);

// gstream stream height tweak
#ifdef GSTREAM_HEIGHT_TWEAK
   fd=open(name_.c_str(),O_WRONLY);
   if(fd<0) {
      fprintf(stderr,"Can't open output file, leaving...\n");
      exit(1);
   }
   if(lseek(fd,0x00b4,SEEK_SET)!=0x00b4) {
      fprintf(stderr,"Can't seek to stream height offset, leaving...\n");
      exit(1);
   }
   if(write(fd,&h,4)!=4) {
      fprintf(stderr,"Can't write new stream height, leaving...\n");
      exit(1);
   }
   close(fd);
#endif
}

// add a frame
bool QCamMovieAvi::addImpl(const QCamFrame & newFrame, const QCam & cam) {

   int out_size,i;
   unsigned char* plan_buf;

   // fill the frame to encode
   fillFrame(newFrame);

   // encode the frame
   out_size=avcodec_encode_video(output_codec_cx,video_outbuf,video_outbuf_size,picture);
   if(out_size>0) {
      AVPacket pkt;
      av_init_packet(&pkt);
      pkt.flags|=AV_PKT_FLAG_KEY;
      pkt.stream_index=output_video_stream->index;
      pkt.data=video_outbuf;
      pkt.size=out_size;
      av_write_frame(output_format_cx,&pkt);
   }
   return true;
}

#endif /* HAVE_LIBAV_H */
