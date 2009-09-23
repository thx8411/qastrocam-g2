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

extern settingsBackup settings;

QCam * QCamV4L::openBestDevice(const char * devpath, const char * devsource) {
   int cam_fd;
   int palette;
   QCam * camFound=NULL;

   if (-1 == (cam_fd=open(devpath,O_RDONLY))) {
      perror(devpath);
      return NULL;
   } else {
      struct video_capability vcap;
      if (ioctl(cam_fd, VIDIOCGCAP,&vcap ) < 0) {
         perror(devpath);
         camFound = NULL;
	 goto exit;
      }
      {
	 // Phillips?
         struct pwc_probe probe;
         int type;
	 bool IsPhilips = false;
	 if (sscanf(vcap.name, "Philips %d webcam", &type) == 1) {
            /* original phillips */
            IsPhilips = true;
         } else if (ioctl(cam_fd, VIDIOCPWCPROBE, &probe) == 0) {
            /* an OEM clone ? */
	    if (!strcmp(vcap.name,probe.name)) {
	       IsPhilips = true;
	       type=probe.type;
            }
         }
	 if (IsPhilips) {
            printf("Philips webcam type %d detected.\n", type);
            close(cam_fd);
            camFound = new QCamVesta(devpath);
	    goto exit;
         }
      }

      if (strncmp(vcap.name,"OV511",5)==0) {
         printf("webcam %s detected.\n",
                vcap.name);
         close(cam_fd);
         camFound = new QCamOV511(devpath);
	 goto exit;
      }
      if (strncmp(vcap.name,"OV519",5)==0) {
         printf("webcam %s detected (jpeg mode).\n",
                vcap.name);
         close(cam_fd);
         camFound = new QCamOV519(devpath);
	 goto exit;
      } 
      printf("unknow %s camera detected.\n"
             "Using generic V4L driver.\n",
             vcap.name);
      close(cam_fd);
      
      if(settings.haveKey("PALETTE")) {
         string palette_=settings.getKey("PALETTE");
         if(strcasecmp(palette_.c_str(),"rgb24")==0) {
            palette=VIDEO_PALETTE_RGB24; 
         } else if(strcasecmp(palette_.c_str(),"yuyv")==0){
            palette=VIDEO_PALETTE_YUYV;
         } else if(strcasecmp(palette_.c_str(),"yuv420p")==0){
            palette=VIDEO_PALETTE_YUV420P;
         } else if(strcasecmp(palette_.c_str(),"yuv420")==0){
            palette=VIDEO_PALETTE_YUV420;
         } else if(strcasecmp(palette_.c_str(),"grey")==0){
            palette=VIDEO_PALETTE_GREY;
         } else {
            palette=0;
         }
      } else { 
         palette=0;
      }
      camFound = new QCamV4L(devpath,palette,devsource);
      goto exit;
   }
exit:
   if (!camFound) printf("no camera detected.\n");
   return camFound;
}
