#include <qpushbutton.h>
#include <time.h>

/** A QPushButton wich need a long "click" to be activated.
 */
class QSlowPushButton : public QPushButton {
   Q_OBJECT
public:
   QSlowPushButton ( int timeSec, const QString & text, QWidget * parent = 0, const char * name = 0) ;
   /// return the actual delay for the click in seconds
   int delay() const;
   /// set the delay needed in second
   void delay(int delay);
private:
   int delay_;
   time_t lastPressed_;
signals:
   void slowClicked();   
private slots:
   void localPressed();
   void localReleased();
};
