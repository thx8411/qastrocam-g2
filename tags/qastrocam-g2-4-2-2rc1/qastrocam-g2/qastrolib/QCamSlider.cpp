#include "QCamSlider.moc"
#include <iostream>

QCamSlider::QCamSlider(const QString & label,bool needCheckBox ,
                       QWidget * parent,int minVal, int maxVal,
                       bool noSliderMove,bool displayPercent):
   QHBox(parent),
   labelTxt_(label) {
   percent_=displayPercent;
   slider_=NULL;
   label_=new QLabel(labelTxt_,this);
   if (needCheckBox) {
      checkBox_=new QCheckBox(this);
      checkBox_->show();
      connect(checkBox_,SIGNAL(toggled(bool)),this,SLOT(buttonToggled(bool)));
      noSliderMove_=noSliderMove;
   } else {
      checkBox_=NULL;
      noSliderMove_=false;
   }
   slider_=new QSlider(QSlider::Horizontal,this);
   slider_->setMinValue(minVal);
   slider_->setMaxValue(maxVal);
   //slider_->setMinValue(minVal);

   slider_->setLineStep((maxVal-minVal)/100);
   if (slider_->lineStep()==0) {
      slider_->setLineStep(1);
   }
   slider_->setPageStep((maxVal-minVal)/10);
   if (slider_->pageStep()==0) {
      slider_->setPageStep(2);
   }
      
   connect(slider_,SIGNAL(sliderMoved(int)),this,SLOT(sliderMove(int)));
   connect(slider_,SIGNAL(valueChanged(int)),this,SLOT(sliderMoveKey(int)));
   valueLabel_= new QLabel(this);
   valueLabel_->setFrameShadow(QFrame::Sunken);
   valueLabel_->show();
   lastEmit_=maxVal+1;

}

void QCamSlider::polish() {
   QHBox::polish();
   if (checkBox_) {
      checkBox_->setChecked(true);
   }
}

void QCamSlider::setValue(int val) {
   if (slider_) {
      lastEmit_=val;
      slider_->setValue(val);
      if (percent_) {
         valueLabel_->setNum(val*100/slider_->maxValue());
      } else {
         valueLabel_->setNum(val);
      }
   }
}

void QCamSlider::sliderMoveKey(int val) {
   if (slider_ && lastEmit_ != val) {
      std::cout << "sliderMoveKey" << std::endl;
      if (checkBox_) {
         checkBox_->setChecked(false);
      }
      lastEmit_=val;
      setValue(val);
      emit valueChange(val);
   }
}

void QCamSlider::sliderMove(int val) {

   if (!slider_) {
      return;
   }

   std::cout << "sliderMove"<<std::endl;
   slider_->setFocus();

   if (checkBox_) {
      checkBox_->setChecked(false);
   }
   lastEmit_=val;
   setValue(val);
   emit valueChange(val);
}

void QCamSlider::buttonToggled(bool val) {
   if (checkBox_) checkBox_->setFocus();
   if (val) {
      emit valueChange(lastEmit_=-1);
      if (noSliderMove_ && slider_) {
         slider_->hide();
         valueLabel_->hide();
      }
   } else {
      if (noSliderMove_ && slider_) {
         slider_->show();
         valueLabel_->show();
         if (percent_) {
            valueLabel_->setNum(slider_->value()*100/slider_->maxValue());
         } else {
            valueLabel_->setNum(slider_->value());
         }
      }
      emit valueChange(lastEmit_=slider_->value());
   }
}

void QCamSlider::setMinValue(int min) {
   slider_->setMinValue(min);
}

void QCamSlider::setMaxValue(int max) {
   slider_->setMaxValue(max);
}
