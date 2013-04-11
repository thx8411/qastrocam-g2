/******************************************************************
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


#ifndef _QTelescopeQHY6_hpp_
#define _QTelescopeQHY6_hpp_

// only available if have usb
#if HAVE_USB_H

#include "QTelescope.hpp"

#include "QHY6cam.hpp"

/** Telescope implementation for the QHY5 ST4 port **/
class QTelescopeQHY6 : public QTelescope {
public:
   QTelescopeQHY6();
   ~QTelescopeQHY6();
   void buildGUI(QWidget * parent);
   virtual int telescopeType() { return(TELESCOPE_QHY6); };
public slots:
   virtual void goE(float shift);
   virtual void goW(float shift);
   virtual void goS(float shift);
   virtual void goN(float shift);
   virtual void stopW();
   virtual void stopE();
   virtual void stopN();
   virtual void stopS();
   virtual double setSpeed(double speed);
   virtual bool setTracking(bool activated);
private:
   QHY6cam* cam_;
};

#endif /* HAVE_USB_H */

#endif
