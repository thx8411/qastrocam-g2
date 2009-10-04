#ifndef _QCamSilder_h_
#define _QCamSilder_h_

#include <qobject.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qslider.h>

class QCamSlider : public QHBox {
   Q_OBJECT;
public:
   QCamSlider(const QString & label,
              bool needCheckBox =false , QWidget * parent = 0 ,
              int minVal=0, int maxVal=65535,bool noSliderMove=false,
              bool displayPercent=true);
   void setMinValue(int min);
   void setMaxValue(int max);
private:
   QString labelTxt_;
   QLabel * label_;
   QLabel * valueLabel_;
   QCheckBox * checkBox_;
   QSlider * slider_;
   bool noSliderMove_;
   bool percent_;
   int lastEmit_;
private slots:
   void sliderMove(int val);
   void sliderMoveKey(int val);
   void buttonToggled(bool state);
   void polish();
 public slots:
   void setValue(int val);
 signals:
   void valueChange(int);
};

#endif
