#ifndef _QCamDisplay_hpp_
#define _QCamDisplay_hpp_

#include "QCamClient.hpp"
#include "QCamFrame.hpp"
#include <qwidget.h>
#include "../config.h"

class QCam;
class QPainter;
class QVBox;
class QCamComboBox;
class QCamDisplayImpl;
class QCamSlider;
class QHBox;
class QLabel;

/** display the frame emeited by the cam.
 */
class QCamDisplay : public QCamClient {
   Q_OBJECT
   void commonInit(QWidget * parent);
public:
   QCamDisplay(QWidget * parent=NULL);
   QCamDisplay(QCam &, QWidget * parent=NULL);
   virtual ~QCamDisplay();
   QWidget & widget();
   QCamFrame yuvFrame() const { return yuvFrame_;}
   enum CrossType {
      None,
      Cross,
      Circle
   };
   enum DisplayMode {
      Color,
      Gray,
      Negate,
      FalseColor
   };
   static const int defaultLum_;
public slots:
   void setCross(CrossType t);
   void setDisplayMode(DisplayMode t);  
protected:
   void camConnected();
protected slots: 
   void newFrame();
   void setCross(int t);
   void setDisplayMode(int t);
   void setCrossLum(int l);
private:
   void setCaption();
   QVBox * mainWidget_;
   QCamDisplayImpl * widget_;
   QHBox * buttonsContainer_;
   QLabel * crossLabel_;
   QCamSlider * crossLumSlider_;
   QCamComboBox * crossButton_;
   QCamComboBox * displayModeButton_;
   QCamFrame yuvFrame_;
   int crossLum_;
#if HAVE_SDL_H
   static bool SDL_on_;
   bool use_SDL_;
   friend class QCamDisplayImplSDL;
#endif
   friend class QCamDisplayImpl;
};

/// helper class for QCamDisplay
class QCamDisplayImpl : public QWidget {
   Q_OBJECT;
protected slots:
   virtual void setCross(QCamDisplay::CrossType);
   virtual void setDisplayMode(QCamDisplay::DisplayMode);
   virtual void setCrossLum(int l);
protected:
   QCamDisplayImpl(QCamDisplay &, QWidget * parent);
   virtual void paintEvent(QPaintEvent * ev)=0;
   virtual void resizeEvent(QResizeEvent*ev);
   QSize sizeHint () const;
   void mouseDoubleClickEvent ( QMouseEvent * e );
   /** annotate the frame with some symbols */
   void annotate(QPainter &);
protected:
   int crossCenterX_;
   int crossCenterY_;
   QCamDisplay & camClient_;
   QCamDisplay::CrossType currentCross_;
   QCamDisplay::DisplayMode displayMode_;
   
   QPainter * painter_;
   QPen * pen_;
   bool firtsFrameReceived_;
   
   friend class QCamDisplay;
};

/** helper class for QCamDisplay.
    Use QT to display images */
class QCamDisplayImplQT : public QCamDisplayImpl {
   Q_OBJECT;
private:
   QCamDisplayImplQT(QCamDisplay &, QWidget * parent);
protected:
   void paintEvent(QPaintEvent * ev);
private:
   friend class QCamDisplay;
};



#include "QCamDisplayImplSDL.hpp"

#endif
