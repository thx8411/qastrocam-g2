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

#include <qmessagebox.h>

#include "QCamQHY5.moc"

QCamQHY5::QCamQHY5() {
   label(QString("QHY5"));
   camera=QHY5cam::instance(QHY_IMAGER);
   if(camera==NULL) {
      QMessageBox::information(0,"Qastrocam-g2","Unable to reach the QHY5 imager\nLeaving...");
      exit(1);
   }
}

QCamQHY5::~QCamQHY5() {
   QHY5cam::destroy(QHY_IMAGER);
}

void QCamQHY5::resize(const QSize & s) {
}

const QSize * QCamQHY5::getAllowedSize() const {
   return sizeTable_;
}

bool QCamQHY5::setSize(int x, int y) {
}


const QSize & QCamQHY5::size() const {
   return yuvBuffer_.size();
}

QWidget * QCamQHY5::buildGUI(QWidget * parent) {
   QWidget * remoteCTRL=QCam::buildGUI(parent);
   return remoteCTRL;
}

