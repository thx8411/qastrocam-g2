/******************************************************************
Qastrocam
Copyright (C) 2003-2009   Franck Sicard
Qastrocam-g2
Copyright (C) 2009-2010   Blaise-Florentin Collin

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


#include "QCamDisplay.hpp"

#if HAVE_SDL_H

#include "QCamDisplayImplSDL.moc"
#include <qpainter.h>
#include <qpen.h>
#include <qmessagebox.h>

#include "yuv.hpp"

/*
 * SDL
 */

#if defined(Q_WS_X11)
#include <X11/Xlib.h>
#endif

inline int min(int a, int b) {
   return (a<=b?a:b);
}

QCamDisplayImplSDL::QCamDisplayImplSDL(QCamDisplay & camClient,QWidget * parent):
         QCamDisplayImpl(camClient,parent),
         screen_(NULL),
         RGBImage_(NULL),
         GreyImage_(NULL) {

   for(int i=0;i<256;i++) {
      greyPalette[i].r=i;
      greyPalette[i].g=i;
      greyPalette[i].b=i;
      negatePalette[255-i].r=i;
      negatePalette[255-i].g=i;
      negatePalette[255-i].b=i;
   }
   for (int i=0;i<256;i+=4) {
      falsePalette[i/4].r=0;
      falsePalette[i/4].g=i;
      falsePalette[i/4].b=255;
      falsePalette[i/4+64].r=0;
      falsePalette[i/4+64].g=255;
      falsePalette[i/4+64].b=255-i;
      falsePalette[i/4+128].r=i;
      falsePalette[i/4+128].g=255;
      falsePalette[i/4+128].b=0;
      falsePalette[i/4+192].r=255;
      falsePalette[i/4+192].g=255-i;
      falsePalette[i/4+192].b=0;
   }

   colors=greyPalette;

   setWFlags(WRepaintNoErase);

   SDL_Init(SDL_INIT_VIDEO);
}

QCamDisplayImplSDL::~QCamDisplayImplSDL() {
   static char variable[64];
   sprintf(variable, "SDL_WINDOWID=0x%lx", winId());
   putenv(variable);
   SDL_FreeSurface(GreyImage_);
   SDL_FreeSurface(RGBImage_);
   SDL_FreeSurface(screen_);
   SDL_QuitSubSystem(SDL_INIT_VIDEO);
   SDL_Quit();
   unsetenv("SDL_WINDOWID");

   QCamDisplay::SDL_on_=false;
}

void QCamDisplayImplSDL::resizeEvent(QResizeEvent*ev) {
   QCamDisplayImpl::resizeEvent(ev);
   if (RGBImage_) {
      SDL_FreeSurface(RGBImage_);
      RGBImage_=NULL;
   }
   if (GreyImage_) {
      SDL_FreeSurface(GreyImage_);
      GreyImage_=NULL;
   }
   if (screen_) {
      SDL_FreeSurface(screen_);
      screen_ = NULL;
   }
   // Set the new video mode with the new window size
   static char variable[64];
   sprintf(variable, "SDL_WINDOWID=0x%lx", winId());
   putenv(variable);
   SDL_QuitSubSystem(SDL_INIT_VIDEO);

   if ( SDL_InitSubSystem(SDL_INIT_VIDEO) < 0 ) {
      QMessageBox::information(0,"Qastrocam-g2","Unable to init SDL\nLeaving...");
      fprintf(stdout, "Unable to init SDL: %s\n", SDL_GetError());
      exit(1);
   }
   screen_ = SDL_SetVideoMode(0/*width()*/, 0/*height()*/, 0 /*32*/, SDL_HWPALETTE | SDL_SWSURFACE | SDL_RLEACCEL);
   if ( ! screen_ ) {
      QMessageBox::information(0,"Qastrocam-g2","Unable to set SDL video mode\nLeaving...");
      fprintf(stdout, "Unable to set video mode: %s\n", SDL_GetError());
      exit(1);
   }
}

void QCamDisplayImplSDL::setDisplayMode(QCamDisplay::DisplayMode mode) {
   QCamDisplayImpl::setDisplayMode(mode);
   setPalette();
}

void QCamDisplayImplSDL::setPalette() {
   if (!GreyImage_) {
      return;
   }
   switch (displayMode_) {
   case QCamDisplay::Color:
      //falling back in grey mode
   case QCamDisplay::Gray:
      colors=greyPalette;
      break;
   case QCamDisplay::Negate:
      colors=negatePalette;
      break;
   case QCamDisplay::FalseColor:
      colors=falsePalette;
      break;
   }
   /* Set palette */
   SDL_SetPalette(GreyImage_, SDL_LOGPAL|SDL_PHYSPAL, colors, 0, 256);
}

void QCamDisplayImplSDL::paintEvent(QPaintEvent * ev) {

   QCamFrame frame=camClient_.yuvFrame();
   if (frame.empty() || screen_==NULL) {
      return;
   }
   SDL_Rect dst;
   dst.x = 0;
   dst.y = 0;
   dst.w = frame.size().width();
   dst.h = frame.size().height();

#if defined(Q_WS_X11)
   // Make sure we're not conflicting with drawing from the Qt library
   XSync(QPaintDevice::x11Display(), FALSE);
#endif

   ImageMode mode=frame.getMode();
   if (displayMode_ != QCamDisplay::Color) {
      mode=GreyFrame;
   }
   switch (mode) {
   case GreyFrame:
      if (GreyImage_ == NULL
          || GreyImage_->w!=frame.size().width()
          || GreyImage_->h!=frame.size().height()) {
         if (GreyImage_) {
            SDL_FreeSurface(GreyImage_);
         }
         GreyImage_=SDL_CreateRGBSurfaceFrom((Uint8*)frame.Y(), frame.size().width(),
                                             frame.size().height(), 8,
                                             frame.size().width(),
                                             0, 0, 0, 0);
         if ( ! GreyImage_ ) {
            fprintf(stderr, "Unable to create grey surface: %s\n", SDL_GetError());
            return;
         }
         setPalette();
      } else {
         GreyImage_->pixels=(Uint8*)frame.Y();
      }
      SDL_BlitSurface(GreyImage_, NULL, screen_, &dst);
      break;
   case YuvFrame:
      if (RGBImage_ == NULL
          || RGBImage_->w!=frame.size().width()
          || RGBImage_->h!=frame.size().height()) {
         Uint32 rmask, gmask, bmask;
         rmask = 0x00FF0000;
         gmask = 0x0000FF00;
         bmask = 0x000000FF;
         if (RGBImage_) {
            SDL_FreeSurface(RGBImage_);
         }
         RGBImage_= SDL_CreateRGBSurface(SDL_SWSURFACE, frame.size().width(),
                                          frame.size().height(), 32,
                                          rmask, gmask, bmask, 0);
         if(RGBImage_ == NULL) {
            QMessageBox::information(0,"Qastrocam-g2","CreateRGBSurface failed\nLeaving...");
            fprintf(stdout, "CreateRGBSurface failed: %s\n", SDL_GetError());
            exit(1);
         }
      }
      yuv444_to_bgr32(RGBImage_->w,RGBImage_->h,frame.Y(),frame.U(),frame.V(),(unsigned char*)RGBImage_->pixels);
      SDL_BlitSurface(RGBImage_, NULL, screen_, &dst);
   }
   SDL_Flip(screen_);

   painter_->begin(this);
   painter_->setPen(*pen_);
   painter_->setClipRegion(ev->region());
   annotate(*painter_);
   painter_->end();
}
#endif
