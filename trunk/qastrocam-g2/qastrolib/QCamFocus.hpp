/******************************************************************
Qastrocam-g2
Copyright (C) 2009-2012   Blaise-Florentin Collin

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


#ifndef _QCAMFOCUS_HPP_
#define _QCAMFOCUS_HPP_

#include <qobject.h>
#include <qstring.h>
#include <qwidget.h>
#include <qvbox.h>
#include <qhgroupbox.h>

//#include <qwtplot3d-qt3/qwt3d_surfaceplot.h>

#include "QCam.hpp"
#include "QCamSelection.hpp"

//
//using namespace Qwt3D;

//class FocusPlot : public SurfacePlot
//{
//  public:
//    FocusPlot();
//};
//

class QCamFocus : public QObject {
   Q_OBJECT
public :
   QCamFocus(QCam* cam);
   ~QCamFocus();
   QWidget* buildGUI(QWidget * parent);
   const QString & label() const;
private slots :
   void focusNewFrame();
   void newSelection();
private :
   QCam* cam_;
   QCamSelection* sel_;
   QVBox* remoteCTRL_;
   QString label_;
};

#endif
