/******************************************************************
Qastrocam
Copyright (C) 2003-2009   Franck Sicard
Qastrocam-g2
Copyright (C) 2009-2012   Blaise-Florentin Collin

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

#include "QCamV4L2.hpp"
#include "QCamV4L2lx.hpp"
#include "QCamV4L2fi.hpp"
#include "QCamOV511.hpp"
#include "QCamVesta.hpp"
#include "QCamDC60.hpp"

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/videodev2.h>
#include "SettingsBackup.hpp"

#define CRITICAL_INTEGRATION_TIME	5.0

// storing settings object.
extern settingsBackup settings;

// Creat the best QCamV4l object depending on the device
QCam * QCamV4L2::openBestDevice(const char * devpath) {
   struct v4l2_capability vcap;
   int cam_fd;
   int palette;
   QCam* camFound=NULL;

   // try to open the device
   if (-1 == (cam_fd=open(devpath,O_RDONLY))) {
      perror(devpath);
      return NULL;
   }


   memset(&vcap,0,sizeof(v4l2_capability));
   // read device cap to get device name
   // if V4L2 api supported
   if (ioctl(cam_fd, VIDIOC_QUERYCAP,&vcap )== 0) {
      char* type;
      bool IsPhilips=false;

      type=(char*)malloc(32);

      if (strcmp((char*)vcap.card, "AstroEasyCap") == 0) {
         cout << "AstroEasyCap driver detected" << endl;
         close(cam_fd);
         camFound = new QCamDC60(devpath);
         return(camFound);
      }

      if (sscanf((char*)vcap.card, "Philips %s webcam", type) == 1) {
         //original phillips
         cout << "Philips webcam type " << type << " detected." << endl;
         IsPhilips = true;
      } else if (strcmp((char*)vcap.driver,"pwc")==0) {
         // if the driver is pwc, we have an OEM clone
         cout << "OEM Philips compatible webcam detected." << endl;
         IsPhilips = true;
      }
      if (IsPhilips) {
         close(cam_fd);

         // temp, just act as a standard V4L2 device, to avoid bus error
         // QCamVesta has to be rewritten
         //camFound = new QCamVesta(devpath);
         //return(camFound);
      }

      free(type);

      // looking for an OV511 device
      if (strncmp((char*)vcap.card,"OV511",5)==0) {
         cout << "webcam " << vcap.card << " detected." << endl;
         close(cam_fd);
         camFound = new QCamOV511(devpath);
         return(camFound);
      }
      // looking for an OV519 device
      if (strncmp((char*)vcap.card,"OV519",5)==0) {
         cout << "webcam " << vcap.card << " detected (jpeg mode)." << endl;
         close(cam_fd);
         camFound = new QCamOV519(devpath);
         return(camFound);
      }
      // V4L2 generic
      int res;
      v4l2_fmtdesc v4l2_fmtdesc_temp;
      v4l2_frmsizeenum v4l2_sizeenum_temp;
      v4l2_format v4l2_fmt_temp;
      v4l2_frmivalenum v4l2_frameinterval;
      // gets the first pix fmt
      memset(&v4l2_fmtdesc_temp,0,sizeof(v4l2_fmtdesc_temp));
      v4l2_fmtdesc_temp.index=0;
      v4l2_fmtdesc_temp.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
      ioctl(cam_fd,VIDIOC_ENUM_FMT,&v4l2_fmtdesc_temp);
      // gets the first size
      memset(&v4l2_sizeenum_temp,0,sizeof(v4l2_sizeenum_temp));
      v4l2_sizeenum_temp.index=0;
      v4l2_sizeenum_temp.pixel_format=v4l2_fmtdesc_temp.pixelformat;
      res=ioctl(cam_fd,VIDIOC_ENUM_FRAMESIZES,&v4l2_sizeenum_temp);
      if(res!=0) {
         // size enum not supported, so frame interval certainly not supported
         cout << "using V4L2 generic" << endl;
         close(cam_fd);
         camFound= new QCamV4L2(devpath);
         return(camFound);
      }
      //
      // probes for FRAMEINTERVALS support
      //
      memset(&v4l2_frameinterval,0,sizeof(v4l2_frameinterval));
      v4l2_frameinterval.index=0;
      v4l2_frameinterval.pixel_format=v4l2_fmtdesc_temp.pixelformat;
      switch(v4l2_sizeenum_temp.type) {
         case V4L2_FRMSIZE_TYPE_DISCRETE :
            // use the first size we found
            v4l2_frameinterval.width=v4l2_sizeenum_temp.discrete.width;
            v4l2_frameinterval.height=v4l2_sizeenum_temp.discrete.height;
            break;
         case V4L2_FRMSIZE_TYPE_CONTINUOUS :
         case V4L2_FRMSIZE_TYPE_STEPWISE :
            // we use the biggest size
            v4l2_frameinterval.width=v4l2_sizeenum_temp.stepwise.max_width;
            v4l2_frameinterval.height=v4l2_sizeenum_temp.stepwise.max_height;
            break;
      }
      if(ioctl(cam_fd,VIDIOC_ENUM_FRAMEINTERVALS,&v4l2_frameinterval)==0) {
         v4l2_frameinterval.index++;
         // frame interval supported
         float max_integration_time=0.0;
         cout << "frame intervals supported" << endl;
         switch(v4l2_frameinterval.type) {
            case V4L2_FRMIVAL_TYPE_DISCRETE :
               cout << "discrete timing" << endl;
               cout << v4l2_frameinterval.discrete.numerator << "/" << v4l2_frameinterval.discrete.denominator << endl;
               max_integration_time=v4l2_frameinterval.discrete.numerator/v4l2_frameinterval.discrete.denominator;
               while(ioctl(cam_fd,VIDIOC_ENUM_FRAMEINTERVALS,&v4l2_frameinterval)==0) {
                  cout << v4l2_frameinterval.discrete.numerator << "/" << v4l2_frameinterval.discrete.denominator << endl;
                  if((v4l2_frameinterval.discrete.numerator/v4l2_frameinterval.discrete.denominator)>max_integration_time)
                     max_integration_time=v4l2_frameinterval.discrete.numerator/v4l2_frameinterval.discrete.denominator;
                  v4l2_frameinterval.index++;
               }
               break;
            case V4L2_FRMIVAL_TYPE_CONTINUOUS :
               cout << "continuous timing" << endl;
            case V4L2_FRMIVAL_TYPE_STEPWISE :
               cout << "stepwise timing" << endl;
               max_integration_time=v4l2_frameinterval.stepwise.max.numerator/v4l2_frameinterval.stepwise.max.denominator;
               break;
         }
         cout << "max integration time : " << max_integration_time << " seconds" << endl;
         if(CRITICAL_INTEGRATION_TIME<max_integration_time) {
            // we use frame intervals for lx
            cout << "Using frame intervals for lx mode" << endl;
            // temp
            close(cam_fd);
            camFound= new QCamV4L2fi(devpath);
            return(camFound);
         } else {
            // we use generic V4L2
            cout << "using V4L2 generic" << endl;
            close(cam_fd);
            camFound= new QCamV4L2lx(devpath);
            return(camFound);
         }
      } else {
         // frame intervals not supported
         cout << "using V4L2 generic" << endl;
         close(cam_fd);
         camFound= new QCamV4L2lx(devpath);
         return(camFound);
      }
   }
   cout << "No camera found" << endl;
   return(NULL);
}
