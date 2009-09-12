#include "QCamOV511.moc"

QCamOV511::QCamOV511(const char * devpath):
   QCamV4L(devpath,0,"",ioNoBlock|ioUseSelect|haveBrightness|haveContrast|haveHue|haveColor) {
   resize(QSize(176,144));
}

void QCamOV511::checkSize(int & x, int & y) const {
   if (x>=capability_.maxwidth && y >= capability_.maxheight) {
      x=capability_.maxwidth;
      y=capability_.maxheight;
      return;
   }
   if (x>=352 && y >=288) {
      x=352;y=288;
      return;
   }
   if (x>=320 && y >=240) {
      x=320;y=240;
      return;
   }
   if (x>=176 && y >=144) {
      x=176;y=144;
      return;
   }
   if (x>=160 && y >=120) {
      x=160;y=120;
      return;
   }
   x=capability_.minwidth;
   y=capability_.minheight;
}

QCamOV519::QCamOV519(const char * devpath):
   QCamV4L(devpath,0,"",DefaultOptions|QCamV4L::sendJpegFrames) {
}
