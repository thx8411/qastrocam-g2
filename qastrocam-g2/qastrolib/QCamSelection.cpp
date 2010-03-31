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


#include "QCamSelection.moc"

#include "QCam.hpp"
#include <math.h>
#include <qcolor.h>
#include <qpen.h>
#include <qpainter.h>
#include "QCamUtilities.hpp"
#include <qvbox.h>
#include <qhbox.h>
#include <qlabel.h>
#include "QCamComboBox.hpp"
#include <qtooltip.h>
#include <math.h>

QCamSelection::QCamSelection(QWidget * parent) : QCamClient(), yuvFrame_() {
   commonInit(parent);
}

QCamSelection::~QCamSelection() {
   mainWidget_->hide();
   delete widget_;
}

QCamSelection::QCamSelection(QCam &theCam,QWidget * parent) :
   QCamClient(theCam),
   yuvFrame_() {
   commonInit(parent);
   setCaption();
}

void QCamSelection::setCaption() {
   QString labelPrefix;
   labelPrefix="Focus ";

   if (isConnected() && mainWidget_) {
      mainWidget_->setCaption(labelPrefix+cam().label());
   }
}

void QCamSelection::commonInit(QWidget * parent) {
   mainWidget_=new QVBox(parent);
   buttonsContainer_ = new QHBox(mainWidget_);

   QWidget* padding1=new QWidget(buttonsContainer_);
   QLabel* label1=new QLabel(buttonsContainer_);
   label1->setText("Palette : ");
   label1->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   buttonsContainer_->setStretchFactor(label1,0);

   int displayValues[]={Gray,Negate,FalseColor};
   const char * displayValuesLabel[]={"Gray","Negated", "False Color"};
   displayModeButton_ = new QCamComboBox(tr("Display type"),buttonsContainer_,3,
                                         displayValues,
                                         displayValuesLabel);
   buttonsContainer_->setStretchFactor(displayModeButton_,0);

   QWidget* padding2=new QWidget(buttonsContainer_);

   QLabel* label2=new QLabel(buttonsContainer_);
   label2->setText("Size : ");
   label2->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   buttonsContainer_->setStretchFactor(label2,0);

   int sizeValues[]={0,1,2};
   const char * sizeValuesLabel[]={"32","64", "128"};
   sizeModeButton_ = new QCamComboBox(tr("Selection size"),buttonsContainer_,3,
                                         sizeValues,
                                         sizeValuesLabel);
   buttonsContainer_->setStretchFactor(sizeModeButton_,0);

   QWidget* padding3=new QWidget(buttonsContainer_);

   connect(displayModeButton_,SIGNAL(change(int)),this,SLOT(setDisplayMode(int)));
   connect(sizeModeButton_,SIGNAL(change(int)),this,SLOT(setSizeMode(int)));

   widget_= new QCamSelectionImpl(*this,mainWidget_);

   QToolTip::add(widget_,"Double click to set center of the selection");

   QSizePolicy policy(QSizePolicy::Expanding,QSizePolicy::Expanding);
   widget_->setSizePolicy(policy);

   QCamUtilities::setQastrocamIcon(mainWidget_);
}

void QCamSelection::newFrame() {
   yuvFrame_=cam().yuvFrame();
   if (yuvFrame_.size() != widget_->size()) {
      widget_->setMinimumSize(yuvFrame_.size());
      widget_->setMaximumSize(yuvFrame_.size());
      widget_->resize(yuvFrame_.size());
      widget_->updateGeometry();
      mainWidget_->adjustSize();
   } else {
      widget_->firtsFrameReceived_=true;
   }
   widget_->update();
}

void QCamSelection::camConnected() {
   widget_->firtsFrameReceived_=false;
   widget_->resize(cam().size());
   widget_->updateGeometry();
   mainWidget_->adjustSize();
   setCaption();
}

QWidget & QCamSelection::widget() {
   return *mainWidget_;
}

QWidget* QCamSelection::impl() {
   return(widget_);
}

void QCamSelection::setDisplayMode(DisplayMode mode) {
   widget_->setDisplayMode(mode);
}

void QCamSelection::setDisplayMode(int t) {
   setDisplayMode((DisplayMode)t);
}

void QCamSelection::setSizeMode(int t) {
   switch(t) {
      case 0 : widget_->setSelectionSize(32); break;
      case 1 : widget_->setSelectionSize(64); break;
      case 2 : widget_->setSelectionSize(128); break;
   }
}

void QCamSelection::setSelectionCenter(int x, int y) {
   widget_->setSelectionCenter(x,y);
}

void QCamSelection::setSelectionSize(int s) {
   setSizeMode(s);
}

int QCamSelection::getSelectionCenterX() {
   return(widget_->getSelectionCenterX());
}

int QCamSelection::getSelectionCenterY() {
   return(widget_->getSelectionCenterY());
}

int QCamSelection::getSelectionSize() {
   return(widget_->getSelectionSize());
}

/*
 * Impl
 */

void QCamSelectionImpl::setDisplayMode(QCamSelection::DisplayMode mode) {
   displayMode_=mode;
}

QCamSelectionImpl::QCamSelectionImpl(QCamSelection & camClient,QWidget * parent):
   QWidget(parent), camClient_(camClient) {
   firtsFrameReceived_=false;
   selectionSize_=32;
   selectionCenterY_=selectionCenterX_=-1000;
   QToolTip::add(this,tr("double click to move center of the selection"));
   painter_ = new QPainter();
   pen_=new QPen();
   pen_->setStyle(SolidLine);
   pen_->setColor(Qt::red);
   displayMode_=QCamSelection::Gray;
   setWFlags(WRepaintNoErase);
}

QCamSelectionImpl::~QCamSelectionImpl() {
   delete painter_;
   delete pen_;
}

void QCamSelectionImpl::mouseDoubleClickEvent ( QMouseEvent * e ) {
   selectionCenterX_=e->x();
   selectionCenterY_=e->y();

   emit selectionChanged();
}

QSize QCamSelectionImpl::sizeHint () const {
   if (camClient_.isConnected()) {
      return camClient_.cam().size();
   } else return QSize(10,10);
}

void QCamSelectionImpl::resizeEvent(QResizeEvent*ev) {
   if (camClient_.isConnected() &&
        firtsFrameReceived_ ) {
      if (size()!=camClient_.cam().size())
         resize(camClient_.cam().size());
   }
}

void QCamSelectionImpl::paintEvent(QPaintEvent * ev) {
   bool changed=false;
   QCamFrame frame=camClient_.yuvFrame();
   if (!frame.empty()) {
      if (selectionCenterX_== -1000) {
         selectionCenterX_=frame.brightestX();
         selectionCenterY_=frame.brightestY();
         changed=true;
      }

      if(selectionSize_>frame.size().width()) {
         selectionSize_=frame.size().width();
         changed=true;
      }

      if(selectionSize_>frame.size().height()) {
         selectionSize_=frame.size().height();
         changed=true;
      }

      if((selectionCenterX_-selectionSize_/2)<0) {
         selectionCenterX_=selectionSize_/2;
         changed=true;
      }
      if((selectionCenterY_-selectionSize_/2)<0) {
         selectionCenterY_=selectionSize_/2;
         changed=true;
      }
      if((selectionCenterX_+selectionSize_/2)>frame.size().width()) {
         selectionCenterX_=frame.size().width()-selectionSize_/2;
         changed=true;
      }
      if((selectionCenterY_+selectionSize_/2)>frame.size().height()) {
         selectionCenterY_=frame.size().height()-selectionSize_/2;
         changed=true;
      }

      if(changed)
         emit selectionChanged();

      int sx=(size().width()-frame.size().width())/2;
      int sy=(size().height()-frame.size().height())/2;
      painter_->begin(this);
      painter_->setPen(*pen_);
      painter_->setClipRegion(ev->region());
      switch(displayMode_) {
      case QCamSelection::Gray:
         painter_->drawImage(sx,sy,frame.grayImage());
         break;
      case QCamSelection::Negate:
         painter_->drawImage(sx,sy,frame.grayImageNegated());
         break;
      case QCamSelection::FalseColor:
         painter_->drawImage(sx,sy,frame.falseColorImage());
         break;
      }
      painter_->drawRect(selectionCenterX_-selectionSize_/2,selectionCenterY_-selectionSize_/2,selectionSize_,selectionSize_);
      painter_->end();
   }
}

void QCamSelectionImpl::setSelectionCenter(int x, int y) {
   selectionCenterX_=x;
   selectionCenterY_=y;

   emit selectionChanged();
}

void QCamSelectionImpl::setSelectionSize(int s) {
   selectionSize_=s;

   emit selectionChanged();
}

int QCamSelectionImpl::getSelectionCenterX() {
   return(selectionCenterX_);
}

int QCamSelectionImpl::getSelectionCenterY() {
   return(selectionCenterY_);
}

int QCamSelectionImpl::getSelectionSize() {
   return(selectionSize_);
}
