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


#ifndef _QCamDisplayImplSDL_hpp_
#define _QCamDisplayImplSDL_hpp_

#if HAVE_SDL_H

#include <SDL.h>

/** helper class for QCamDisplay.
    Use SDL to display images. */

class QCamDisplayImplSDL : public QCamDisplayImpl {
private:
   QCamDisplayImplSDL(QCamDisplay &, QWidget * parent);
   virtual ~QCamDisplayImplSDL();
protected:
   void paintEvent(QPaintEvent * ev);
   void resizeEvent(QResizeEvent*ev);
   void setPalette();
private:
   SDL_Color* colors;
   SDL_Color  greyPalette[256];
   SDL_Color  negatePalette[256];
   SDL_Color  falsePalette[256];

   SDL_Surface *screen_;
   SDL_Surface * RGBImage_;
   SDL_Surface * GreyImage_;
   friend class QCamDisplay;
  protected slots:
    virtual void setDisplayMode(QCamDisplay::DisplayMode);
 };
#endif

#endif
