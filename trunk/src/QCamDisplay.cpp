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
#include "QCamDisplayImplSDL.hpp"

#include "QCam.hpp"
//Added by qt3to4:
#include <QtGui/QPaintEvent>
#include <QtGui/QResizeEvent>
#include <QtGui/QMouseEvent>
#include <math.h>
#include <Qt/qcolor.h>
#include <Qt/qpen.h>
#include <Qt/qpainter.h>
#include "QCamUtilities.hpp"
#include <Qt3Support/q3vbox.h>
#include <Qt3Support/q3hbox.h>
#include <Qt/qlabel.h>
#include "QCamSlider.hpp"
#include "QCamComboBox.hpp"
#include <Qt/qtooltip.h>
#include <math.h>

const int QCamDisplay::defaultLum_=255;

#if HAVE_SDL_H
bool QCamDisplay::SDL_on_=false;
#endif

QCamDisplay::QCamDisplay(QWidget * parent) :
   QCamClient(),
   yuvFrame_() {
   commonInit(parent);
}

QCamDisplay::~QCamDisplay() {
   view_->hide();

   QCamUtilities::removeWidget(view_);

   delete widget_;
}

QCamDisplay::QCamDisplay(QCam &theCam,QWidget * parent) :
   QCamClient(theCam),
   yuvFrame_() {
   commonInit(parent);
   //widget_->resize(theCam.size());
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
   mainWidget_=new Q3VBox(parent);

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
   //policy.setHorStretch(100);
   //policy.setVerStretch(100);
   widget_->setSizePolicy(policy);

   view_ = new Q3ScrollView(parent);
   view_->setResizePolicy(Q3ScrollView::AutoOne);
   view_->addChild(mainWidget_);

   QCamUtilities::setQastrocamIcon(view_);
   QCamUtilities::registerWidget(view_);

   view_->show();
}

void QCamDisplay::newFrame() {
   yuvFrame_=cam().yuvFrame();
   if (yuvFrame_.size() != widget_->size()) {
      QSize viewSize;
      //widget_->setMinimumSize(yuvFrame_.size());
      //widget_->setMaximumSize(yuvFrame_.size());
      //cout << "newFrame resize()\n";
      widget_->resize(yuvFrame_.size());
      widget_->updateGeometry();
      crossButton_->updateGeometry();
      mainWidget_->adjustSize();
      view_->adjustSize();
      viewSize=mainWidget_->sizeHint();
      viewSize=QSize(viewSize.width()+view_->horizontalScrollBar()->sizeHint().width(),viewSize.height()+view_->verticalScrollBar()->sizeHint().height());
      //view_->setGeometry(0,0,viewSize.width(),viewSize.height());
      view_->setMaximumSize(viewSize);
   } else {
      widget_->firtsFrameReceived_=true;
   }
   widget_->update();
}

void QCamDisplay::camConnected() {
   QSize viewSize;
   //cout << "connected cam : "<< cam().size().width() <<"x"<<cam().size().height()<<"\n";
   widget_->firtsFrameReceived_=false;
   widget_->resize(cam().size());
   widget_->updateGeometry();
   crossButton_->updateGeometry();
   mainWidget_->adjustSize();
   view_->adjustSize();
   viewSize=mainWidget_->sizeHint();
   viewSize=QSize(viewSize.width()+view_->horizontalScrollBar()->sizeHint().width(),viewSize.height()+view_->verticalScrollBar()->sizeHint().height());
   //view_->setGeometry(0,0,viewSize.width(),viewSize.height());
   view_->setMaximumSize(viewSize);
   setCaption();
}

QWidget & QCamDisplay::widget() {
   return *view_;
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

QCamDisplayImpl::QCamDisplayImpl(QCamDisplay & camClient,QWidget * parent):
   QWidget(parent), camClient_(camClient) {
   firtsFrameReceived_=false;
   currentCross_=QCamDisplay::None;
   crossCenterY_=crossCenterX_=-1000;
   QToolTip::add(this,tr("double click to move center of the reticul"));
   painter_ = new QPainter();
   pen_=new QPen();
   pen_->setStyle(Qt::DotLine);
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
   if (camClient_.isConnected() &&
        firtsFrameReceived_ ) {
      //cout << "resize display "<<size().width()<<"x"<<size().height()<<"\n";
      //cout << "old size was:"<<ev->oldSize().width()<<"x"<<ev->oldSize().height()<<"\n";
            //cout << "camsize "<< camClient_.cam().size().width()<<"x"<< camClient_.cam().size().height()<<"\n";
      //resize(camClient_.cam().size());
      //crossCenterY_=crossCenterX_=-1000;
      if (size()!=camClient_.cam().size())
         resize(camClient_.cam().size());
   }
}

void QCamDisplayImpl::setCrossLum(int l) {
   pen_->setColor(QColor(l,0,0));
}

/*
 * QT
 */
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

   //int sx=(size().width()-frame.size().width())/2;
   //int sy=(size().height()-frame.size().height())/2;

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
