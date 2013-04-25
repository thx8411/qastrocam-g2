/******************************************************************
Qastrocam
Copyright (C) 2003-2009   Franck Sicard
Qastrocam-g2
Copyright (C) 2010-2013   Blaise-Florentin Collin

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


#ifndef _QCamSelection_hpp_
#define _QCamSelection_hpp_

#include <Qt/qwidget.h>
#include <Qt/qlabel.h>
#include <QtGui/QPaintEvent>
#include <QtGui/QResizeEvent>
#include <QtGui/QMouseEvent>

#include "QCamVBox.hpp"
#include "QCamClient.hpp"
#include "QCamFrame.hpp"

class QCam;
class QPainter;
class QCamComboBox;
class QCamSelectionImpl;
class Q3HBox;
class QLabel;

/* display the frame emited by the cam */
class QCamSelection : public QCamClient {
   Q_OBJECT
   void commonInit(QWidget* parent);
public:
   QCamSelection(QWidget* parent=NULL);
   QCamSelection(QCam &, QWidget* parent=NULL);
   virtual ~QCamSelection();
   QWidget& widget();
   QWidget* impl();
   QCamFrame yuvFrame() const { return yuvFrame_;}
   enum DisplayMode {
      Gray,
      Negate,
      FalseColor
   };
   void setSelectionCenter(int x, int y);
   void setSelectionSize(int s);
   int getSelectionCenterX();
   int getSelectionCenterY();
   int getSelectionSize();
public slots:
   void setDisplayMode(DisplayMode t);
protected:
   void camConnected();
protected slots:
   void newFrame();
   void setDisplayMode(int t);
   void setSizeMode(int t);
private:
   void setCaption();
   QCamVBox* mainWidget_;
   QCamSelectionImpl* widget_;
   Q3HBox* buttonsContainer_;
   QCamComboBox* displayModeButton_;
   QCamComboBox* sizeModeButton_;
   QCamFrame yuvFrame_;
   friend class QCamDisplayImpl;
};

// helper class for QCamDisplay
class QCamSelectionImpl : public QWidget {
   Q_OBJECT
protected slots:
   virtual void setDisplayMode(QCamSelection::DisplayMode);
   void setSelectionCenter(int x, int y);
   void setSelectionSize(int s);
   int getSelectionCenterX();
   int getSelectionCenterY();
   int getSelectionSize();
protected:
   QCamSelectionImpl(QCamSelection &, QWidget* parent);
   ~QCamSelectionImpl();
   virtual void paintEvent(QPaintEvent* ev);
   virtual void resizeEvent(QResizeEvent* ev);
   QSize sizeHint () const;
   void mouseDoubleClickEvent ( QMouseEvent* e );
   int selectionSize_;
   int selectionCenterX_;
   int selectionCenterY_;
   QCamSelection & camClient_;
   QCamSelection::DisplayMode displayMode_;
   QPainter* painter_;
   QPen* pen_;
   bool firtsFrameReceived_;
   friend class QCamSelection;
signals :
   void selectionChanged();
};

#endif
