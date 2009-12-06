#ifndef _QCamMoviAvi_hpp_
#define _QCamMoviAvi_hpp_

#include "../config.h"

#if HAVE_AVIFILE_H

#include "QCamMovie.hpp"

#include <avifile.h>

class QCamMovieAvi : public QCamMovie {
public:
   QCamMovieAvi();
   ~QCamMovieAvi();
   QWidget * buildGUI(QWidget  * father);
   bool openImpl(const string & seqName, const QCam & cam);
   void closeImpl();
   bool addImpl(const QCamFrame & newFrame, const QCam & cam);
private:
   mutable avm::IWriteFile *aviFile_;
   mutable avm::IVideoWriteStream *aviStream_;
   mutable unsigned char *deinterlaceBuf_;
};

#endif

#endif
