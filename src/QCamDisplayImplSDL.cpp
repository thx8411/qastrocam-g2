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

// do we have the SDL library ?
#if HAVE_SDL_H

#include <math.h>

#include <Qt/qpainter.h>
#include <Qt/qpen.h>
#include <Qt/qmessagebox.h>
#include <QtGui/QResizeEvent>
#include <QtGui/QPaintEvent>

#include "yuv.hpp"

#include "QCam.hpp"
#include "QCamDisplay.hpp"
#include "QCamDisplayImplSDL.hpp"


// needed for Xsync
#if defined(Q_WS_X11)
#include <X11/Xlib.h>
#include <QtGui/qx11info_x11.h>
#endif

//
// helpers for SDL drawing
//

// sets a pixel
// CAUTION : it doesn't check borders !!
void SDL_DrawPixel(SDL_Surface *screen, int x, int y, Uint8 R, Uint8 G, Uint8 B) {
   Uint32 color = SDL_MapRGB(screen->format, R, G, B);

   switch (screen->format->BytesPerPixel) {
      case 1: { /* Assuming 8-bpp */
            Uint8 *bufp;
            bufp = (Uint8 *)screen->pixels + y*screen->pitch + x;
            *bufp = color;
         }
         break;

      case 2: { /* Probably 15-bpp or 16-bpp */
            Uint16 *bufp;
            bufp = (Uint16 *)screen->pixels + y*screen->pitch/2 + x;
            *bufp = color;
         }
         break;

      case 3: { /* Slow 24-bpp mode, usually not used */
            Uint8 *bufp;
            bufp = (Uint8 *)screen->pixels + y*screen->pitch + x;
            *(bufp+screen->format->Rshift/8) = R;
            *(bufp+screen->format->Gshift/8) = G;
            *(bufp+screen->format->Bshift/8) = B;
         }
         break;

      case 4: { /* Probably 32-bpp */
            Uint32 *bufp;
            bufp = (Uint32 *)screen->pixels + y*screen->pitch/4 + x;
            *bufp = color;
          }
          break;
   }
}

// draws a circle
void SDL_DrawCircle(SDL_Surface *surface, int n_cx, int n_cy, int radius, Uint8 R, Uint8 G, Uint8 B) {

   // if the first pixel in the screen is represented by (0,0) (which is in sdl)
   // remember that the beginning of the circle is not in the middle of the pixel
   // but to the left-top from it:

   double error = (double)-radius;
   double x = (double)radius -0.5;
   double y = (double)0.5;
   double cx = n_cx - 0.5;
   double cy = n_cy - 0.5;

   while (x >= y) {
      SDL_DrawPixel(surface, (int)(cx + x), (int)(cy + y), R, G, B);
      SDL_DrawPixel(surface, (int)(cx + y), (int)(cy + x), R, G, B);

      if (x != 0) {
         SDL_DrawPixel(surface, (int)(cx - x), (int)(cy + y), R, G, B);
         SDL_DrawPixel(surface, (int)(cx + y), (int)(cy - x), R, G, B);
      }

      if (y != 0) {
         SDL_DrawPixel(surface, (int)(cx + x), (int)(cy - y), R, G, B);
         SDL_DrawPixel(surface, (int)(cx - y), (int)(cy + x), R, G, B);
      }

      if (x != 0 && y != 0) {
         SDL_DrawPixel(surface, (int)(cx - x), (int)(cy - y), R, G, B);
         SDL_DrawPixel(surface, (int)(cx - y), (int)(cy - x), R, G, B);
      }

      error += y;
      ++y;
      error += y;

      if (error >= 0) {
         --x;
         error -= x;
         error -= x;
      }
   }
}

//
// class implementation
//

QCamDisplayImplSDL::QCamDisplayImplSDL(QCamDisplay & camClient,QWidget * parent):
         QCamDisplayImpl(camClient,parent),
         screen_(NULL),
         RGBImage_(NULL),
         GreyImage_(NULL) {

   // filling the grey and nagtive palettes
   for(int i=0;i<256;i++) {
      greyPalette[i].r=i;
      greyPalette[i].g=i;
      greyPalette[i].b=i;
      negatePalette[255-i].r=i;
      negatePalette[255-i].g=i;
      negatePalette[255-i].b=i;
   }

   // filling the false color palette
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

   // default is grey palette
   colors=greyPalette;

   // disable Qt4 double buffering
   setAttribute(Qt::WA_NoBackground);
   setAttribute(Qt::WA_PaintOnScreen);
   setAttribute(Qt::WA_NoSystemBackground);

   // init attribute
   crossLum_=0xFF;

   // SDL init
   if(SDL_WasInit(SDL_INIT_VIDEO)==0)
      SDL_InitSubSystem(SDL_INIT_VIDEO);
}

QCamDisplayImplSDL::~QCamDisplayImplSDL() {
   // SDL_WINDOWID trick
   static char variable[64];
   sprintf(variable, "SDL_WINDOWID=0x%lx", winId());
   putenv(variable);

   // release SDL stuff
   SDL_FreeSurface(GreyImage_);
   SDL_FreeSurface(RGBImage_);
   SDL_FreeSurface(screen_);

   // unset SDL_WINDOWID trick
   unsetenv("SDL_WINDOWID");

   // SDL is off
   QCamDisplay::SDL_on_=false;

   SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void QCamDisplayImplSDL::resizeEvent(QResizeEvent*ev) {
   QCamDisplayImpl::resizeEvent(ev);

   // size changed, release SDL surfaces
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

   // SDL_WINDOWID trick
   static char variable[64];
   sprintf(variable, "SDL_WINDOWID=0x%lx", winId());
   putenv(variable);

   // reset SDL
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
   // get the frame
   QCamFrame frame=camClient_.yuvFrame();

   // sanity check
   if (frame.empty() || screen_==NULL) {
      return;
   }

   // SDL clip zone
   SDL_Rect dst;
   dst.x = 0;
   dst.y = 0;
   dst.w = frame.size().width();
   dst.h = frame.size().height();

#if defined(Q_WS_X11)
   // Make sure we're not conflicting with drawing from the Qt library
   XSync(QX11Info::display(), FALSE);
#endif

   // build SDL surfaces depending on video mode
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

   // annotates the surface
   // we annotate directly on the SDL surface. When Qt draws on the
   // surface, we get frame flickering

   // lock the surface
   SDL_LockSurface(screen_);

   // default center
   if (crossCenterX_== -1000) {
      crossCenterX_=size().width()/2;
      crossCenterY_=size().height()/2;
   }

   // draw the sight
   switch (currentCross_) {
      // cross display
      case QCamDisplay::Cross:
         for(int i=0; i<width(); i++)
            SDL_DrawPixel(screen_, i, crossCenterY_, (Uint8)crossLum_, 0x00, 0x00);
         for(int j=0; j<height(); j++)
            SDL_DrawPixel(screen_, crossCenterX_, j, (Uint8)crossLum_, 0x00, 0x00);
         break;
      // sight display
      case QCamDisplay::Circle: {
            int step=height()/10;
            int max=min(min(crossCenterX_,width()-crossCenterX_),min(crossCenterY_,height()-crossCenterY_));
            for (int i=step/2;i<max;i+=step)
               SDL_DrawCircle(screen_, crossCenterX_, crossCenterY_, i, (Uint8)crossLum_, 0x00, 0x00);
         }
         break;
   }

   // small annotation (King method)
   if (camClient_.cam().annotationEnabled_) {
      int x=(int)round(camClient_.cam().annotationPos_.x());
      int y=(int)round(camClient_.cam().annotationPos_.y());
      if (x<0) x=4;
      else if (x>=camClient_.cam().size().width()) {
         x=camClient_.cam().size().width()-1-4;
      }
      if (y<0) y=4;
      else if (y>=camClient_.cam().size().height()) {
         y=camClient_.cam().size().height()-1-4;
      }
      for(int i=x-10;i<x+10;i++)
         SDL_DrawPixel(screen_, i, y, (Uint8)crossLum_, 0x00, 0x00);
      for(int j=y-10;j<y+10;j++)
         SDL_DrawPixel(screen_, x, j, (Uint8)crossLum_, 0x00, 0x00);
   }

   // unlock the surface
   SDL_UnlockSurface(screen_);

   // display the frame
   SDL_Flip(screen_);
}



#endif
