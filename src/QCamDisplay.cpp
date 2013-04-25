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

#include <math.h>

#include <Qt/qcolor.h>
#include <Qt/qpen.h>
#include <Qt/qpainter.h>
#include <Qt/qlabel.h>
#include <Qt/qtooltip.h>
#include <QtGui/QPaintEvent>
#include <QtGui/QResizeEvent>
#include <QtGui/QMouseEvent>

#include <Qt3Support/q3hbox.h>

#include "QCamVBox.hpp"
#include "QCamUtilities.hpp"
#include "QCam.hpp"
#include "QCamSlider.hpp"
#include "QCamComboBox.hpp"
#include "QCamDisplay.hpp"
#include "QCamDisplayImplSDL.hpp"


#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

// default minimum display size
#define	DISPLAY_BASE_WIDTH	480
#define	DISPLAY_BASE_HEIGHT	360

const int QCamDisplay::defaultLum_=255;

#if HAVE_SDL_H
bool QCamDisplay::SDL_on_=false;
#endif


//
// class QCamDisplay
//

QCamDisplay::QCamDisplay(QWidget * parent) :
   QCamClient(),
   yuvFrame_() {
   commonInit(parent);
}

QCamDisplay::~QCamDisplay() {
   mainWidget_->hide();

   QCamUtilities::removeWidget(mainWidget_);

   delete widget_;
}

QCamDisplay::QCamDisplay(QCam &theCam,QWidget * parent) :
   QCamClient(theCam),
   yuvFrame_() {
   commonInit(parent);
   setCaption();
}

void QCamDisplay::setCaption() {
   QString labelPrefix;
#if HAVE_SDL_H
   if (use_SDL_) {
      labelPrefix="SDL ";
   } else
#endif
   {
      labelPrefix="QT ";
   }

   if (isConnected() && view_) {
      view_->setCaption(labelPrefix+cam().label());
   }
}

void QCamDisplay::commonInit(QWidget * parent) {
   mainWidget_=new QCamVBox(parent);

   buttonsContainer_ = new Q3HBox(mainWidget_);

   int displayValues[]={Color,Gray,Negate,FalseColor};
   const char * displayValuesLabel[]={"RGB","Gray","Negated", "False Color"};
   displayModeButton_ = new QCamComboBox(tr("Display type"),buttonsContainer_,4,
                                         displayValues,
                                         displayValuesLabel);
   connect(displayModeButton_,SIGNAL(change(int)),this,SLOT(setDisplayMode(int)));

   crossLabel_= new QLabel("Reticle : ",buttonsContainer_);
   crossLabel_->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

   int crossValues[]={None,Cross,Circle};
   const char * crossValuesLabel[]={"none","cross","circle"};
   crossButton_ = new QCamComboBox("Cross type",buttonsContainer_,3,
                                   crossValues,crossValuesLabel);
   connect(crossButton_,SIGNAL(change(int)),this,SLOT(setCross(int)));

   crossLumSlider_ = new QCamSlider("Lum.",false,mainWidget_,10,255,false,false);
   QToolTip::add(crossLumSlider_,"set the brightness of the reticule");
   crossLumSlider_->setEnabled(false);
#if HAVE_SDL_H
   if (!SDL_on_ && QCamUtilities::useSDL()) {
      SDL_on_=true;
      use_SDL_=true;
      widget_= new QCamDisplayImplSDL(*this,mainWidget_);
   } else {
      widget_= new QCamDisplayImplQT(*this,mainWidget_);
      use_SDL_=false;
   }
#else
   widget_= new QCamDisplayImplQT(*this,mainWidget_);
#endif
   QToolTip::add(widget_,"Double click to set center of the reticule");
   connect(crossLumSlider_,SIGNAL(valueChange(int)),this,SLOT(setCrossLum(int)));
   crossLumSlider_->setValue(QCamDisplay::defaultLum_);

   QSizePolicy policy(QSizePolicy::Expanding,QSizePolicy::Expanding);
   widget_->setSizePolicy(policy);

   // scroll area
   view_ = new QScrollArea(mainWidget_);
   // frames aren't resisable
   view_->setWidgetResizable(FALSE);
   // center the frame
   view_->setAlignment(Qt::AlignCenter);
   // frames are in the scroll area
   view_->setWidget(widget_);

   // registering widget for night vision mode
   QCamUtilities::setQastrocamIcon(mainWidget_);
   QCamUtilities::registerWidget(mainWidget_);

   // display
   mainWidget_->show();
}

void QCamDisplay::newFrame() {
   yuvFrame_=cam().yuvFrame();
   // size changed ?
   if(yuvFrame_.size() != widget_->size()) {
      int h,w;
      // resize the frame and informs parents
      widget_->resize(yuvFrame_.size());
      widget_->updateGeometry();
      // size is limited (screen can be small)
      if(yuvFrame_.size().height()>DISPLAY_BASE_HEIGHT)
         h=DISPLAY_BASE_HEIGHT;
      else
         h=yuvFrame_.size().height();
      if(yuvFrame_.size().width()>DISPLAY_BASE_WIDTH)
         w=DISPLAY_BASE_WIDTH;
      else
         w=yuvFrame_.size().width();
      // update the scroll area
      view_->setMinimumSize(QSize(w+16,h+16));
      // center the scroll if needed
      view_->ensureVisible(yuvFrame_.size().width()/2,yuvFrame_.size().height()/2,w/2,h/2);
      // informs parents
      view_->updateGeometry();
      // adjust parent's size
      mainWidget_->adjustSize();
   } else {
      widget_->firtsFrameReceived_=true;
   }
   widget_->update();
}

void QCamDisplay::camConnected() {
   int h,w;
   widget_->firtsFrameReceived_=false;
   // update the frame and informs parents
   widget_->resize(cam().size());
   widget_->updateGeometry();
   //  size is limited (screen can be small)
   if(cam().size().height()>DISPLAY_BASE_HEIGHT)
      h=DISPLAY_BASE_HEIGHT;
   else
      h=cam().size().height();
   if(cam().size().width()>DISPLAY_BASE_WIDTH)
      w=DISPLAY_BASE_WIDTH;
   else
      w=cam().size().width();
   // update the scroll area
   view_->setMinimumSize(QSize(w+16,h+16));
   // center the frame if needed
   view_->ensureVisible(cam().size().width()/2,cam().size().height()/2,w/2,h/2);
   // informs parents
   view_->updateGeometry();
   // adjuste parent's size
   mainWidget_->adjustSize();

   setCaption();
}

QWidget & QCamDisplay::widget() {
   return *mainWidget_;
}

void QCamDisplay::setDisplayMode(DisplayMode mode) {
   widget_->setDisplayMode(mode);
}

void QCamDisplayImpl::setDisplayMode(QCamDisplay::DisplayMode mode) {
   displayMode_=mode;
}

void QCamDisplay::setDisplayMode(int v) {
   setDisplayMode((DisplayMode)v);
}

void QCamDisplay::setCross(QCamDisplay::CrossType cross) {
   widget_->setCross(cross);
   crossLumSlider_->setEnabled(cross!= QCamDisplay::None);
   crossButton_->update(cross);
}

void QCamDisplay::setCross(int v) {
   setCross((CrossType)v);
}

void QCamDisplay::setCrossLum(int l) {
   crossLum_=l;
   widget_->setCrossLum(l);
}


//
// class QCamDisplayImpl
//

QCamDisplayImpl::QCamDisplayImpl(QCamDisplay & camClient,QWidget * parent):
   QWidget(parent), camClient_(camClient) {
   firtsFrameReceived_=false;
   currentCross_=QCamDisplay::None;
   crossCenterY_=crossCenterX_=-1000;
   QToolTip::add(this,tr("double click to move center of the reticul"));
   painter_ = new QPainter();
   pen_=new QPen();
   pen_->setStyle(Qt::SolidLine);
   pen_->setColor(QColor(QCamDisplay::defaultLum_,0,0));
   displayMode_=QCamDisplay::Color;
}

QCamDisplayImpl::~QCamDisplayImpl() {
   delete painter_;
   delete pen_;
}

void QCamDisplayImpl::mouseDoubleClickEvent ( QMouseEvent * e ) {
   crossCenterX_=e->x();
   crossCenterY_=e->y();
   if (currentCross_ == QCamDisplay::None) {
      camClient_.setCross(QCamDisplay::Cross);
   }
}

QSize QCamDisplayImpl::sizeHint () const {
   if (camClient_.isConnected()) {
      return camClient_.cam().size();
   } else return QSize(10,10);
}

void QCamDisplayImpl::setCross(   QCamDisplay::CrossType cross) {
   currentCross_=cross;
   if (currentCross_== QCamDisplay::None) {
      crossCenterY_=crossCenterX_=-1000;
   }
}

void QCamDisplayImpl::resizeEvent(QResizeEvent*ev) {
   if (camClient_.isConnected() && firtsFrameReceived_ ) {
      if (size()!=camClient_.cam().size())
         resize(camClient_.cam().size());
   }
}

void QCamDisplayImpl::setCrossLum(int l) {
   crossLum_=l;
   pen_->setColor(QColor(l,0,0));
}


//
// class QCamDisplayImplQT
//

QCamDisplayImplQT::QCamDisplayImplQT(QCamDisplay & camClient,QWidget * parent):
   QCamDisplayImpl(camClient,parent) {
   setWindowFlags(Qt::WNoAutoErase);
}

void QCamDisplayImplQT::paintEvent(QPaintEvent * ev) {
   QCamFrame frame=camClient_.yuvFrame();
   if (!frame.empty()) {
      if (crossCenterX_== -1000) {
         crossCenterX_=size().width()/2;
         crossCenterY_=size().height()/2;
      }
      int sx=(size().width()-frame.size().width())/2;
      int sy=(size().height()-frame.size().height())/2;
      painter_->begin(this);
      painter_->setPen(*pen_);
      painter_->setClipRegion(ev->region());
      switch(displayMode_) {
      case QCamDisplay::Color:
         painter_->drawImage(sx,sy,frame.colorImage());
         break;
      case QCamDisplay::Gray:
         painter_->drawImage(sx,sy,frame.grayImage());
         break;
      case QCamDisplay::Negate:
         painter_->drawImage(sx,sy,frame.grayImageNegated());
         break;
      case QCamDisplay::FalseColor:
         painter_->drawImage(sx,sy,frame.falseColorImage());
         break;
      }
      annotate(*painter_);
      painter_->end();
   }
}

void QCamDisplayImpl::annotate(QPainter & painter) {
   QCamFrame frame=camClient_.yuvFrame();

   if (crossCenterX_== -1000) {
      crossCenterX_=size().width()/2;
      crossCenterY_=size().height()/2;
   }

   switch (currentCross_) {
   case QCamDisplay::Cross:
      painter.drawLine(crossCenterX_,0,
                       crossCenterX_,height());
      painter.drawLine(0,crossCenterY_,
                       width(),crossCenterY_);
      break;
   case QCamDisplay::Circle: {
      int step=height()/10;
      int max=min(min(crossCenterX_,width()-crossCenterX_),min(crossCenterY_,height()-crossCenterY_));
      for (int i=step/2;i<max;i+=step) {
         painter.drawEllipse(crossCenterX_-i,crossCenterY_-i,2*i,2*i);
      }
   }
      break;
   case QCamDisplay::None:
      break;
   }

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
      painter.drawLine(x,y-10,x,y+10);
      painter.drawLine(x-10,y,x+10,y);
   }
}
