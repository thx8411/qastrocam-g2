/******************************************************************
Qastrocam-g2
Copyright (C) 2010 Blaise-Florentin Collin

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


#include "../config.h"

#include <stdlib.h>

#include "QCamMovieSer.hpp"
#include "QCamFrame.hpp"
#include "QCam.hpp"
#include "SettingsBackup.hpp"

// settings object, needed everywhere
extern settingsBackup settings;

QCamMovieSer::QCamMovieSer() {

   //
   // to do
   //

}

QCamMovieSer::~QCamMovieSer() {

   //
   // to do
   //

}

QWidget * QCamMovieSer::buildGUI(QWidget  * father) {
   // nothing to do yet
   return father;
}

bool QCamMovieSer::openImpl(const string & seqName, const QCam & cam) {

   //
   // to do
   //
   cerr << "Open SER" << endl;

   return true;
}

void QCamMovieSer::closeImpl() {

   //
   // to do
   //
   cerr << "Close SER" << endl;

}

bool QCamMovieSer::addImpl(const QCamFrame & newFrame, const QCam & cam) {

   //
   // to do
   //
   cerr << "Add SER" << endl;

   return true;
}
