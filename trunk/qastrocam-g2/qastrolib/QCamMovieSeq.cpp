#include "QCamMovieSeq.hpp"
#include "QCam.hpp"

#include <sys/stat.h>
#include <sys/types.h>

QCamMovieSeq::QCamMovieSeq() {
}

QWidget * QCamMovieSeq::buildGUI(QWidget  * father) {
   return father;
}

bool QCamMovieSeq::openImpl(const string & seqName, const QCam & cam) {
   seqenceFileName_=seqName;
   if (mkdir(seqenceFileName_.c_str(),0777)) {
      perror("QCamMovieSeq::open");
      return false;
   }
   return true;
}

void QCamMovieSeq::closeImpl() {
   seqenceFileName_="/dev/null";
}

bool QCamMovieSeq::addImpl(const QCamFrame & newFrame, const QCam & cam) {
   char number[10];
   snprintf(number,9,"%d",getFrameNumber());
   return cam.saveFrame(seqenceFileName_+'/'+number);
}
