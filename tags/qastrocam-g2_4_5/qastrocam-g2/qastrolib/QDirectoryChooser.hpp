#ifndef _QDirectoryChooser_hpp_
#define _QDirectoryChooser_hpp_

#include <qpushbutton.h>
#include <qobject.h>

class QDirectoryChooser : public QPushButton {
   Q_OBJECT;
   QString currentDir_;
public:
   QDirectoryChooser(QWidget * parent=0);
   ~QDirectoryChooser();
private slots:
   void selectDirectory();
   void setDirectory(const QString &);
signals:
   void directoryChanged(const QString &);
};

#endif
