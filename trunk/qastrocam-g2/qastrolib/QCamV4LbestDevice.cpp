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


#include "QCamV4L.hpp"
#include "QCamOV511.hpp"
#include "QCamVesta.hpp"

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/videodev.h>
#include "SettingsBackup.hpp"

// storing settings object.
extern settingsBackup settings;

// Creat the best QCamV4l object depending on the device.
// Usage of default object can be done using "force"
QCam * QCamV4L::openBestDevice(const char * devpath, const char * devsource, bool force) {
   int cam_fd;
   int palette;
   QCam * camFound=NULL;

   // try to open the device
   if (-1 == (cam_fd=open(devpath,O_RDONLY))) {
      perror(devpath);
      return NULL;
   }
   // read device cap to get device name
   struct v4l2_capability vcap;
   if (ioctl(cam_fd, VIDIOC_QUERYCAP,&vcap ) < 0) {
      perror(devpath);
      camFound = NULL;
      cout << "no camera detected." << endl;
      return camFound;
   }
   // if V4L generic usage not forced
   if(!force) {
      // is the device a phillips cam ?
      struct pwc_probe probe;
      int type;
      bool IsPhilips = false;
      if (sscanf((char*)vcap.card, "Philips %d webcam", &type) == 1) {
         /* original phillips */
         IsPhilips = true;
      } else if (ioctl(cam_fd, VIDIOCPWCPROBE, &probe) == 0) {
         /* an OEM clone ? */
         if (!strcmp((char*)vcap.card,probe.name)) {
            IsPhilips = true;
            type=probe.type;
         }
      }
      if (IsPhilips) {
         cout << "Philips webcam type " << type << " detected." << endl;
         close(cam_fd);
         camFound = new QCamVesta(devpath);
         return camFound;
      }
      // looking for an OV511 device
      if (strncmp((char*)vcap.card,"OV511",5)==0) {
         cout << "webcam " << vcap.card << " detected." << endl;
         close(cam_fd);
         camFound = new QCamOV511(devpath);
         return camFound;
      }
      // looking for an OV519 device
      if (strncmp((char*)vcap.card,"OV519",5)==0) {
         cout << "webcam " << vcap.card << " detected (jpeg mode)." << endl;
         close(cam_fd);
         camFound = new QCamOV519(devpath);
         return camFound;
      }
      cout << "unknow camera detected." << endl;
   } else
      cout << "V4L generic forced" << endl;
   // using generic V4L
   cout << "Using generic V4L driver : " << vcap.card << endl;
   close(cam_fd);

   camFound = new QCamV4L(devpath,devsource);
   return camFound;
}

