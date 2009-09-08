#include "QCamDisplay.hpp"

#if HAVE_SDL_H

#include "QCamDisplayImplSDL.moc"
#include <qpainter.h>
#include <qpen.h>

#include "bayer.hpp"

/*
 * SDL
 */

#include <SDL.h>
#if defined(Q_WS_X11)
#  include <X11/Xlib.h>
#endif

inline int min(int a, int b) {
   return (a<=b?a:b);
}

QCamDisplayImplSDL::QCamDisplayImplSDL(QCamDisplay & camClient,QWidget * parent):
   QCamDisplayImpl(camClient,parent),
   screen_(NULL),
   YUVImage_(NULL),
   RGBImage_(NULL),
   GreyImage_(NULL) {
   setWFlags(WRepaintNoErase);
}

QCamDisplayImplSDL::~QCamDisplayImplSDL() {
   static char variable[64];
   sprintf(variable, "SDL_WINDOWID=0x%lx", winId());
   putenv(variable);
   SDL_QuitSubSystem(SDL_INIT_VIDEO);
   unsetenv("SDL_WINDOWID");

   QCamDisplay::SDL_on_=false;
}

void QCamDisplayImplSDL::resizeEvent(QResizeEvent*ev) {
   QCamDisplayImpl::resizeEvent(ev);
      
   // We could get a resize event at any time, so clean previous mode
   screen_ = NULL;
      
   // Set the new video mode with the new window size
   static char variable[64];
   sprintf(variable, "SDL_WINDOWID=0x%lx", winId());
   putenv(variable);
   SDL_QuitSubSystem(SDL_INIT_VIDEO);

   if ( SDL_InitSubSystem(SDL_INIT_VIDEO) < 0 ) {
      fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
      return;
   }
   screen_ = SDL_SetVideoMode(width(), height(), 0 /*32*/, SDL_SWSURFACE);
   if ( ! screen_ ) {
      fprintf(stderr, "Unable to set video mode: %s\n", SDL_GetError());
      return;
   }

   if (YUVImage_) {
      //SDL_FreeYUVOverlay(YUVImage_);
      YUVImage_=NULL;
   }
   if (RGBImage_) {
      //SDL_FreeSurface(RGBImage_);
      RGBImage_=NULL;
   }

   if (GreyImage_) {
      //SDL_FreeYUVOverlay(GreyImage_);
      GreyImage_=NULL;
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
   SDL_Color colors[256];
   switch (displayMode_) {
   case QCamDisplay::Color:
      //falling back in grey mode
   case QCamDisplay::Gray:
      for(int i=0;i<256;i++) {
         colors[i].r=i;
         colors[i].g=i;
         colors[i].b=i;
      }
      break;
   case QCamDisplay::Negate:
      for(int i=0;i<256;i++) {
         colors[255-i].r=i;
         colors[255-i].g=i;
         colors[255-i].b=i;
      }
      break;
   case QCamDisplay::FalseColor:
      for (int i=0;i<256;i+=4) {
         colors[i/4].r=0;
         colors[i/4].g=i;
         colors[i/4].b=255;
      }
      for (int i=0;i<256;i+=4) {
         colors[i/4+64].r=0;
         colors[i/4+64].g=255;
         colors[i/4+64].b=255-i;
      }
      for (int i=0;i<256;i+=4) {
         colors[i/4+128].r=i;
         colors[i/4+128].g=255;
         colors[i/4+128].b=0;
      }
      for (int i=0;i<256;i+=4) {
         colors[i/4+192].r=255;
         colors[i/4+192].g=255-i;
         colors[i/4+192].b=0;
      }
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
            //SDL_FreeYUVOverlay(GreyImage_);
         }
         GreyImage_=SDL_CreateRGBSurfaceFrom((Uint8*)frame.Y(), frame.size().width(),
                                             frame.size().height(), 8,
                                             frame.size().width(),
                                             0, 0, 0, 0);
         if ( ! GreyImage_ ) {
            fprintf(stderr, "Unable to create grey overlay: %s\n", SDL_GetError());
            return;
         }
         setPalette();
      } else {
         GreyImage_->pixels=(Uint8*)frame.Y();
      }
      SDL_BlitSurface(GreyImage_, NULL, screen_, &dst);
      //cout << "overlay "<<(int)(GreyImage_->pixels)<<" "<<(int)(GreyImage_->pixels[0])<<" "<<(int)(GreyImage_->pixels[1])<<" "<<(int)(GreyImage_->pixels[2])<<"\n"<<flush;
      
      break;
   case YuvFrame: 
      if (YUVImage_ == NULL
          || YUVImage_->w!=frame.size().width()
          || YUVImage_->h!=frame.size().height()) {
         if (YUVImage_) {
            //SDL_FreeYUVOverlay(YUVImage_);
         }
         YUVImage_=SDL_CreateYUVOverlay(frame.size().width(),frame.size().height(),
                                        SDL_IYUV_OVERLAY,screen_);
         if ( ! YUVImage_ ) {
            fprintf(stderr, "Unable to create YUV overlay: %s\n", SDL_GetError());
            return;
         }
      }
      
      SDL_LockYUVOverlay(YUVImage_);
      
      YUVImage_->pixels[0]=(Uint8*)frame.Y();
      YUVImage_->pixels[1]=(Uint8*)frame.U();
      YUVImage_->pixels[2]=(Uint8*)frame.V();
      
      //YUVImage_->pixels=data;
      SDL_UnlockYUVOverlay(YUVImage_);
      if (SDL_DisplayYUVOverlay(YUVImage_, &dst)) {
         fprintf(stderr, "Unable to display YUV overlay %s.\n",SDL_GetError());
      }
      break;
   case RawRgbFrame1:
   case RawRgbFrame2:
   case RawRgbFrame3:
   case RawRgbFrame4:
      if (RGBImage_ == NULL
          || RGBImage_->w!=frame.size().width()
          || RGBImage_->h!=frame.size().height()) {
         
         Uint32 rmask, gmask, bmask;
         
         rmask = 0x00FF0000;
         gmask = 0x0000FF00;
         bmask = 0x000000FF;
         
         RGBImage_ = SDL_CreateRGBSurface(SDL_SWSURFACE, frame.size().width(),
                                          frame.size().height(), 32,
                                          rmask, gmask, bmask, 0);
         if(RGBImage_ == NULL) {
            fprintf(stderr, "CreateRGBSurface failed: %s\n", SDL_GetError());
            exit(1);
         }
      }
      raw2rgb((unsigned char*)RGBImage_->pixels,
              frame.Y(),RGBImage_->w,RGBImage_->h,frame.getMode());
      SDL_BlitSurface(RGBImage_, NULL, screen_, &dst);
   }
   
   //SDL_FillRect(screen_, NULL, 0);
   SDL_Flip(screen_);

#if 0
   if (crossCenterX_== -1000) {
      crossCenterX_=size().width()/2;
      crossCenterY_=size().height()/2;
   }
#endif
   painter_->begin(this);
   painter_->setPen(*pen_);
   painter_->setClipRegion(ev->region());
   
   annotate(*painter_);
   
   painter_->end();
}
#endif
