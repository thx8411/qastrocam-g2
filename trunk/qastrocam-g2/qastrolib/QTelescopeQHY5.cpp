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


#include "QTelescopeQHY5.moc"

#include <iostream>
#include <string.h>

#include "SettingsBackup.hpp"

using namespace std;

extern settingsBackup settings;

QTelescopeQHY5::QTelescopeQHY5(const char * pport) : QTelescope() {

   //

   stopE();
   stopW();
   stopN();
   stopS();
}

QTelescopeQHY5::~QTelescopeQHY5() {

   //

}

void QTelescopeQHY5::goE(float shift) {
   stopW();

   //

}

void QTelescopeQHY5::goW(float shift) {
   stopE();

   //

}

void QTelescopeQHY5::goS(float shift) {
   stopN();

   //

}

void QTelescopeQHY5::goN(float shift) {
   stopS();

   //

}

void QTelescopeQHY5::stopE() {

   //

}

void QTelescopeQHY5::stopN() {

   //

}

void QTelescopeQHY5::stopW() {

   //

}

void QTelescopeQHY5::stopS() {

   //

}

double QTelescopeQHY5::setSpeed(double speed) {
   return(0.0);
}

bool QTelescopeQHY5::setTracking(bool activated) {
   // always tracking ?
   return activated;
}
