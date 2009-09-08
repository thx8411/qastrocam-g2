#ifndef _QCamOV511_hpp_
#define _QCamOV511_hpp_

#include "QCamV4L.hpp"
/** enhance QCamV4L to handle specifities of OV511 webcams. */

class QCamOV511 : public QCamV4L {
   Q_OBJECT;
public:
   QCamOV511(const char * devpath="/dev/video0");
protected:
   /** update x and y to a valid size */
   virtual void checkSize(int & x, int & y) const;
};

class QCamOV519 : public QCamV4L {
   Q_OBJECT;
public:
   QCamOV519(const char * devpath="/dev/video0");
protected:
};
#endif
