/******************************************************************
Qastrocam-g2
Copyright (C) 2013   Blaise-Florentin Collin

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

#include "QCamWindow.hpp"

// constructor
QCamWindow::QCamWindow(QWidget* parent, const char* name, Qt::WFlags f):QWidget(parent, f) {
}

// destructor
QCamWindow::~QCamWindow() {
}

// close event caption
void QCamWindow::closeEvent(QCloseEvent *event)
{
   emit(windowClosed());
   event->accept();
}
