/******************************************************************
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


#ifndef _QTelescopeQHY5_hpp_
#define _QTelescopeQHY5_hpp_

#include "QTelescope.hpp"

#include "QHY5cam.hpp"

/** Telescope implementation for the QHY5 ST4 port **/
class QTelescopeQHY5 : public QTelescope {
   Q_OBJECT;
public:
   QTelescopeQHY5();
   ~QTelescopeQHY5();
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
   QHY5cam* cam_;
};

#endif
