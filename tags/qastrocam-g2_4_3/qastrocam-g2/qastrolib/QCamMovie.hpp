#ifndef _QCAMMOVIE_HPP_
#define _QCAMMOVIE_HPP_

#include <qobject.h>
#include <string>
#include <stdio.h>

using namespace std;

class QCamFrame;
class QCam;
class QString;
class QWidget;

class QCamMovie {
public:
   QCamMovie() {};
   virtual ~QCamMovie() {};
   virtual QWidget * buildGUI(QWidget  * father) { return father;}
   bool open(const string & seqName, const QCam & cam);
   void close();
   bool add(const QCamFrame & newFrame, const QCam & cam);
   int getFrameNumber() const { return frameNumber_; }
protected:
   virtual bool openImpl(const string & seqName, const QCam & cam)=0;
   virtual void closeImpl()=0;
   virtual bool addImpl(const QCamFrame & newFrame, const QCam & cam)=0;
private:
   FILE * propFile_;
   int frameNumber_;
};

#endif
