/******************************************************************
Qastrocam-g2
Copyright (C) 2010   Blaise-Florentin Collin

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


#ifndef _QTelescopeSimulator_hpp_
#define _QTelescopeSimulator_hpp_

#include <string>

#include <qlabel.h>
#include <qpushbutton.h>
#include <qvgroupbox.h>

#include "QTelescope.hpp"

using namespace std;

class QTelescopeSimulator : public QTelescope {
   Q_OBJECT;
public:
   QTelescopeSimulator();
   ~QTelescopeSimulator();
   virtual int telescopeType() { return(TELESCOPE_SIMULATOR); }
   void buildGUI(QWidget * parent);
private:
   QVGroupBox* simulatorWidget_;
   QLabel* speedLabel_;
   QPushButton* buttonUp_;
   QPushButton* buttonDown_;
   QPushButton* buttonLeft_;
   QPushButton* buttonRight_;
public slots:
   virtual void goE(float shift);
   virtual void goW(float shift);
   virtual void goS(float shift);
   virtual void goN(float shift);
   virtual void stopE();
   virtual void stopN();
   virtual void stopW();
   virtual void stopS();
   virtual double setSpeed(double speed);
   virtual bool setTracking(bool activated);
};

#endif
