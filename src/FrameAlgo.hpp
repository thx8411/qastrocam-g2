/******************************************************************
Qastrocam
Copyright (C) 2003-2009   Franck Sicard
Qastrocam-g2
Copyright (C) 2009   Blaise-Florentin Collin

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


#ifndef _FrameAlgo_hpp_
#define _FrameAlgo_hpp_

#include <Qt/qobject.h>

#include "QCamTrans.hpp"
#include "QCamFrame.hpp"

class QWidget;

/* transform frames */
class FrameAlgo : public QObject {
   Q_OBJECT
public:
   /* Main callback to implement.
       Transfor the frame in. result is stored in out.
       \return true if there is a result, if false out is not modified.
   */
   virtual bool transform(const QCamFrame in, QCamFrame & out) =0;
   virtual QString label() const =0;
   virtual QWidget * allocGui(QWidget * parent) const=0;
   virtual ~FrameAlgo() {}

public:
   /* Forgot any state from the previous frames. */
   virtual void reset() {}
protected :
   QCamTrans* cam_;
};

#endif
