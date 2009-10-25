#ifndef _QCamMovieSeq_hpp_
#define _QCamMovieSeq_hpp_

#include "QCamMovie.hpp"

class QCamMovieSeq : public QCamMovie {
public:
   QCamMovieSeq();
   QWidget * buildGUI(QWidget  * father);
   bool openImpl(const string & seqName, const QCam & cam);
   void closeImpl();
   bool addImpl(const QCamFrame & newFrame, const QCam & cam);
private:
   int fileSeqenceNumber_;
   string seqenceFileName_;
};

#endif
