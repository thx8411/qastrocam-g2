#include "QCamDisplay.moc"

#include "QCam.hpp"
#include <math.h>
#include <qcolor.h>
#include <qpen.h>
#include <qpainter.h>
#include "QCamUtilities.hpp"
#include <qvbox.h>
#include <qhbox.h>
#include <qlabel.h>
#include "QCamSlider.hpp"
#include "QCamComboBox.hpp"
#include <qtooltip.h>
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
   mainWidget_->hide();
   delete mainWidget_;
   //delete widget_;
   //delete buttonsContainer_;
   //delete crossLabel_;
   //delete crossLumSlider_;
   //delete crossButton_;
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

   if (isConnected() && mainWidget_) {
      mainWidget_->setCaption(labelPrefix+cam().label());
   }
}

void QCamDisplay::commonInit(QWidget * parent) {
   mainWidget_=new QVBox(parent);
   buttonsContainer_ = new QHBox(mainWidget_);

   int displayValues[]={Color,Gray,Negate,FalseColor};
   const char * displayValuesLabel[]={"RGB","Gray","Negated", "False Color"};
   displayModeButton_ = new QCamComboBox(tr("Cross type"),buttonsContainer_,4,
                                         displayValues,
                                         displayValuesLabel);
   connect(displayModeButton_,SIGNAL(change(int)),this,SLOT(setDisplayMode(int)));

   crossLabel_= new QLabel("Reticle : ",buttonsContainer_);
   crossLabel_->setAlignment(AlignRight|AlignVCenter);

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

   QCamUtilities::setQastrocamIcon(mainWidget_);
   //mainWidget_->show();
}

void QCamDisplay::newFrame() {
   yuvFrame_=cam().yuvFrame();
   if (yuvFrame_.size() != widget_->size()) {
      widget_->setMinimumSize(yuvFrame_.size());
      widget_->setMaximumSize(yuvFrame_.size());
      //cout << "newFrame resize()\n";
      widget_->resize(yuvFrame_.size());
      widget_->updateGeometry();
      crossButton_->updateGeometry();
      mainWidget_->adjustSize();
      //mainWidget_->updateGeometry();
   } else {
      widget_->firtsFrameReceived_=true;
   }
   widget_->update();
}

void QCamDisplay::camConnected() {
   //cout << "connected cam : "<< cam().size().width() <<"x"<<cam().size().height()<<"\n";
   widget_->firtsFrameReceived_=false;
   widget_->resize(cam().size());
   widget_->updateGeometry();
   //crossButton_->updateGeometry();
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

QCamDisplayImpl::QCamDisplayImpl(QCamDisplay & camClient,QWidget * parent):
   QWidget(parent), camClient_(camClient) {
   firtsFrameReceived_=false;
   crossCenterY_=crossCenterX_=-1000;
   QToolTip::add(this,tr("double click to move center of the reticul"));
   painter_ = new QPainter();
   pen_=new QPen();
   pen_->setStyle(DotLine);
   pen_->setColor(QColor(QCamDisplay::defaultLum_,0,0));
   displayMode_=QCamDisplay::Color;
}

void QCamDisplayImpl::mouseDoubleClickEvent ( QMouseEvent * e ) {
   crossCenterX_=e->x();
   crossCenterY_=e->y();
   if (currentCross_ == QCamDisplay::None) {
#if 0
      currentCross_=QCamDisplay::Cross;
      camClient_.crossButton_->update(currentCross_);
#else
      camClient_.setCross(QCamDisplay::Cross);
#endif
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
#if 0
   if (camClient_.isConnected() &&
       (camClient_.cam().size() != size() &&
        firtsFrameReceived_ )) {
      cout << "resize display "<<size().width()<<"x"<<size().height()<<"\n";
      cout << "* old size was:"<<ev->oldSize().width()<<"x"<<ev->oldSize().height()<<"\n";
      camClient_.cam().resize(size());
      
      //cout << "camsize "<< camClient_.cam().size().width()<<"x"<< camClient_.cam().size().height()<<"\n";
      //resize(camClient_.cam().size());
      //crossCenterY_=crossCenterX_=-1000;
      //setMinimumSize(camClient_.cam().size());
      //setMaximumSize(camClient_.cam().size());
      updateGeometry();
      parentWidget()->adjustSize();
   }
#else
   if (camClient_.isConnected() &&
        firtsFrameReceived_ ) {
      cout << "resize display "<<size().width()<<"x"<<size().height()<<"\n";
      cout << "* old size was:"<<ev->oldSize().width()<<"x"<<ev->oldSize().height()<<"\n";
            //cout << "camsize "<< camClient_.cam().size().width()<<"x"<< camClient_.cam().size().height()<<"\n";
      //resize(camClient_.cam().size());
      //crossCenterY_=crossCenterX_=-1000;
      if (size()==camClient_.cam().size()) {
#if 0
         setMinimumSize(camClient_.cam().size());
         setMaximumSize(camClient_.cam().size());
         updateGeometry();
         parentWidget()->adjustSize();
#endif
      } else {
         resize(camClient_.cam().size());
      }
   }
#endif
}

void QCamDisplayImpl::setCrossLum(int l) {
   pen_->setColor(QColor(l,0,0));
}

/*
 * QT
 */
QCamDisplayImplQT::QCamDisplayImplQT(QCamDisplay & camClient,QWidget * parent):
   QCamDisplayImpl(camClient,parent) {
   setWFlags(WRepaintNoErase);
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
#if 0
      switch (currentCross_) {
      case QCamDisplay::Cross:
         painter_->drawLine(crossCenterX_,0,
                            crossCenterX_,height());
         painter_->drawLine(0,crossCenterY_,
                            width(),crossCenterY_);
         break;
      case QCamDisplay::Circle: {
            int step=height()/10;
            int max=min(min(crossCenterX_,width()-crossCenterX_),min(crossCenterY_,height()-crossCenterY_));
            for (int i=step/2;i<max;i+=step) {
               painter_->drawEllipse(crossCenterX_-i,crossCenterY_-i,2*i,2*i);
            }
         }
         break;
      case QCamDisplay::None:
         break;
      }
#endif
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

