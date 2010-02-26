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

#include <sys/ioctl.h>

#include "QCamDC60.moc"

#include "private_ioctl.h"

QCamDC60::QCamDC60(const char * devpath):
   QCamV4L2(devpath,ioNoBlock|ioUseSelect|haveBrightness|haveContrast|haveHue|haveColor) {
   resize(QSize(176,144));
}

void QCamDC60::setGPSW(bool b) {
   struct v4l2_control ctrl;
   ctrl.id=V4L2_CID_GPSW1;
   ctrl.value=b;

   if(ioctl(device_,VIDIOC_S_CTRL,&ctrl)!=0)
      cout << "Unable to change GPSW state" << endl;
}
