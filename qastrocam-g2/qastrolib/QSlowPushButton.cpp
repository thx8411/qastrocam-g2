#include "QSlowPushButton.moc"
#include <qtooltip.h>

QSlowPushButton::QSlowPushButton(int theDelay, const QString & text, QWidget * parent, const char * name) :
   QPushButton(text, parent,name) {
   connect( this, SIGNAL(pressed()), this, SLOT(localPressed()) );
   connect( this, SIGNAL(released()), this, SLOT(localReleased()) );
   delay(theDelay);
}

int QSlowPushButton::delay() const {
   return delay_;
}

void QSlowPushButton::delay(int delay) {
   delay_=delay;
   QToolTip::add(this,QString().sprintf("Maintain pressed '%d' seconds to activate",delay_));
}

void QSlowPushButton::localPressed() {
   lastPressed_=time(&lastPressed_);
}

void QSlowPushButton::localReleased() {
   time_t newTime;
   newTime=time(&newTime);
   if ((newTime-lastPressed_)>=delay_) emit (slowClicked());
}
