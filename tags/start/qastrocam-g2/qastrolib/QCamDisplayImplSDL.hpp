#ifndef _QCamDisplayImplSDL_hpp_
#define _QCamDisplayImplSDL_hpp_

#if HAVE_SDL_H

struct SDL_Surface;
struct SDL_Overlay;
/** helper class for QCamDisplay.
    Use SDL to display images. */

class QCamDisplayImplSDL : public QCamDisplayImpl {
   Q_OBJECT;
private:
   QCamDisplayImplSDL(QCamDisplay &, QWidget * parent);
   virtual ~QCamDisplayImplSDL();
protected:
   void paintEvent(QPaintEvent * ev);
   void resizeEvent(QResizeEvent*ev);
   void setPalette();
private:
   SDL_Surface *screen_;
   SDL_Overlay *YUVImage_;
   SDL_Surface * RGBImage_;
   SDL_Surface * GreyImage_;
   friend class QCamDisplay;
  protected slots:
    virtual void setDisplayMode(QCamDisplay::DisplayMode);
 };
#endif
#endif
