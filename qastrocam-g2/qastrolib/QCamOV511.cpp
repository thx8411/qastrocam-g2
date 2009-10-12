#include "QCamOV511.moc"

QCamOV511::QCamOV511(const char * devpath):
   QCamV4L(devpath,0,"",ioNoBlock|ioUseSelect|haveBrightness|haveContrast|haveHue|haveColor) {
   resize(QSize(176,144));
}

QCamOV519::QCamOV519(const char * devpath):
   QCamV4L(devpath,0,"",DefaultOptions|QCamV4L::sendJpegFrames) {
}
