#ifndef _FrameAlgo_hpp_
#define _FrameAlgo_hpp_

#include "QCamFrame.hpp"
#include <qobject.h>

class QWidget;

/** transform frames */
class FrameAlgo : public QObject {
   Q_OBJECT;
public:
   /** Main callback to implement.
       Transfor the frame in. result is stored in out.
       \return true if there is a result, if false out is not modified.
   */
   virtual bool transform(const QCamFrame in, QCamFrame & out) =0;
   virtual QString label() const =0;
   virtual QWidget * allocGui(QWidget * parent) const=0;
   virtual ~FrameAlgo() {}
   
public:
   /** Forgot any state from the previous frames. */
   virtual void reset() {}
};

#endif
