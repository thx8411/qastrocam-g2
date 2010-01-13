/******************************************************************
Qastrocam
Copyright (C) 2003-2009   Franck Sicard
Qastrocam-g2
Copyright (C) 2009-2010 Blaise-Florentin Collin

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License v2
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
MA  02110-1301, USA.
*******************************************************************/


#include "QCamAdd.moc"

#include "QCamSlider.hpp"
#include <stdio.h>
#include <qpushbutton.h>
#include <qprogressbar.h>
#include <qcheckbox.h>
#include <qvgroupbox.h>
#include <qhgroupbox.h>

#include <stdlib.h>
#include <math.h>

#include "QCamRadioBox.hpp"
#include "QCamComboBox.hpp"

const int QCamAdd::numOfBuffers_=512;

void QCamAdd::removeFrame(const QCamFrame & frame) {
   int dummyMin,dummyMax,dummyCr;
   moveFrame(frame,dummyMin,dummyMax,dummyCr,false);
}

void QCamAdd::addFrame(const QCamFrame & frame,
                       int & maxYValue,
                       int & minYValue,
                       int & maxCrValue) {
   moveFrame(frame,maxYValue,minYValue,maxCrValue,true);
}

void QCamAdd::moveFrame(const QCamFrame & frame,
                        int & maxYValue,
                        int & minYValue,
                        int & maxCrValue,
                        const bool adding) {
   int size;
   int tmpValue;
   maxYValue=minYValue=integrationBuff_[0];
   int * dest=integrationBuff_;

   const uchar * src=frame.Y();
   size=frame.ySize();
   for (int i=0;i<size;++i) {
      if (adding) {
         tmpValue=((*dest)+=(*src));
         if (tmpValue>maxYValue) {
            maxYValue=tmpValue;
         } else if (tmpValue<minYValue && tmpValue !=0) {
            minYValue=tmpValue;
         }
      } else {
         *dest-=*src;
      }
      ++dest;
      ++src;
   }

   if (mode_==YuvFrame) {
      maxCrValue=0;
      src=frame.U();
      size=frame.uSize();
      for (int i=0;i<size;++i) {
         if (adding) {
            tmpValue=abs((*dest)+=(-128+(*src)));
            if (tmpValue>maxCrValue_) {
               maxCrValue=tmpValue;
            }
         } else {
            *dest-=(-128+*src);
         }
         ++dest;
         ++src;
      }
      src=frame.V();
      size=frame.vSize();
      for (int i=0;i<size;++i) {
         if (adding) {
            tmpValue=abs((*dest)+=(-128+(*src)));
            if (tmpValue>maxCrValue_) {
               maxCrValue=tmpValue;
            }
         } else {
            *dest-=(-128+*src);
         }
         ++dest;
         ++src;
      }
      switch (maxCrValueAutoSaturated_) {
      case 0:
         maxCrValue=100000;
         break;
      case 1:
         maxCrValue=maxYValue;
         break;
      case 2:
         maxCrValue=maxYValue>>1;
         break;
      case 3:
         maxCrValue=((maxYValue>>1)+maxCrValue)>>1;
         break;
      case 4:
         //maxCrValue=maxCrValue;
         break;
      default:
         // invalid case
         cerr << "invalid value "<< maxCrValueAutoSaturated_ << "for maxCrValueAutoSaturated_\n";
      }
   } else {
      maxCrValue=maxYValue>>1;
   }
}

QCamAdd::QCamAdd(QCam* cam) :
   curSize_(0,0) {
   int nbuf=numOfBuffers_;
   curBuff_=0;
   numOfActiveBuffers_=1;
   numOfActivatedBuffers_=16;
   cam_=cam;
   integrationBuff_=NULL;
   newIntegrationBuff_=false;
   maxYValue_=0;
   minYValue_=0;
   maxYValueAuto_=true;
   minYValueAuto_=true;
#ifdef QCAM_ADD_COLOR
   maxCrValueAutoSaturated_=false;
   maxCrValue_=0;
#endif
   funcDisplay_=NULL;
   negateDisplay_=false;
   frameHistory_=new QCamFrame[nbuf];
   allocBuff(cam_->size());
   connect(cam_,SIGNAL(newFrame()),this,SLOT(addNewFrame()));
#ifndef QCAM_ADD_COLOR
   setGray(true);
#endif
   label(tr("Stacking"));
}

QCamAdd::~QCamAdd() {
   //clear();
   free(integrationBuff_);
   //delete frameHistory_;
}

void QCamAdd::integration2yuv(const int * integration,
                              QCamFrame & yuv) const {
   int i;
   int Ysize=yuv.ySize();
   int value;
   uchar * yB, * uB, * vB;
#ifdef QCAM_ADD_COLOR
   int Usize=yuv.uSize();
   int Vsize=yuv.vSize();
#endif
   yuv.setMode(mode_);
   yB=yuv.YforUpdate();
   if (funcDisplay_) {
      double dInterval=funcDisplay_(maxYValue_)-funcDisplay_(minYValue_);
      double logMin=funcDisplay_(minYValue_);
      for (i=0;i<Ysize;++i) {
         //value=(integration[i]-minYValue_)*255/interval;
         if (integration[i]>0) {
            value=(int)rint((funcDisplay_(integration[i])-logMin)*255/dInterval);
            if (value <= 0) {
               yB[i]=0;
            } else if (value >= 255) {
               yB[i]=255;
            } else {
               yB[i]=value;
            }
         } else {
            yB[i]=0;
         }
      }
   } else {
      int interval=maxYValue_-minYValue_;
      if (interval<=0) {
         interval=1;
      }

      for (i=0;i<Ysize;++i) {
         value=(integration[i]-minYValue_)*255/interval;
         if (value <= 0) {
            yB[i]=0;
         } else if (value >= 255) {
            yB[i]=255;
         } else {
            yB[i]=value;
         }
      }
   }

   if (negateDisplay_) {
      for (i=0;i<Ysize;++i) {
         yB[i]=255-yB[i];
      }
   }

   if (mode_==YuvFrame) {
      uB=yuv.UforUpdate();
      for (i=0;i<Usize;++i) {
         //value=(integration[i+Ysize]-shiftCr)*128/maxCrValue;
         value=integration[i+Ysize]*128/maxCrValue_;
         if (value <= -128) {
            uB[i]=0;
         } else if (value >= 127) {
            uB[i]=255;
         } else {
            uB[i]=value+128;
         }
      }

      vB=yuv.VforUpdate();
      for (i=0;i<Vsize;++i) {
         //value=(integration[i+Ysize+Usize]-shiftCr)*128/maxCrValue;
         value=integration[i+Ysize+Usize]*128/maxCrValue_;
         if (value <= -128) {
            vB[i]=0;
         } else if (value >= 127) {
            vB[i]=255;
         } else {
            vB[i]=value+128;
         }
      }
   }
}

QCamFrame QCamAdd::yuvFrame() const {
   if (newIntegrationBuff_) {
      integration2yuv(integrationBuff_,computedFrame_);
      newIntegrationBuff_=false;
   }

   //computedFrame_=cam_->yuvFrame();

   return computedFrame_;
}

void QCamAdd::addNewFrame() {
   addFrame(cam_->yuvFrame());
   /*
   if (!computedFrame_.empty()) {
      emit (newFrame());
   }
   */
}


void QCamAdd::zeroBuff(const QSize & size) {
   memset(integrationBuff_,0,sizeof(int) * size.height()*size.width()*3);
   for (int i=0;i<numOfActivatedBuffers_;++i) {
      frameHistory_[i].clear();
   }
}

void QCamAdd::allocBuff(const QSize & size) {
   free(integrationBuff_);
   integrationBuff_=(int*)malloc(size.height()*size.width()*3*sizeof(int));
   computedFrame_.setSize(size);

   for (int i=0;i<numOfBuffers_;++i) {
      frameHistory_[i].setSize(size);
   }

   zeroBuff(size);
}

void QCamAdd::addFrame(const QCamFrame & frame) {
   if (curSize_ != cam_->size()) {
      allocBuff( cam_->size());
      curSize_= cam_->size();
      resetBufferFill();
   }
   switch (frame.getMode()) {
   case GreyFrame:
/*   case RawRgbFrame1:
   case RawRgbFrame2:
   case RawRgbFrame3:
   case RawRgbFrame4:
      mode_=frame.getMode();*/
      mode_=GreyFrame;
      break;
   case YuvFrame:
      mode_=(maxCrValueAutoSaturated_==0)?GreyFrame:YuvFrame;
   }

   //frameHistory_[curBuff_]->removeFrame(integrationBuff_);
   removeFrame(frameHistory_[curBuff_]);

   if (maxYValueAuto_) {
      maxYValue_=-256*256;
   }
   if (minYValueAuto_) {
      minYValue_=256*256;
   }

   frameHistory_[curBuff_]=frame;

   int tmpMax,tmpMin;
   addFrame(frameHistory_[curBuff_],tmpMax,tmpMin,maxCrValue_);

   if (maxYValueAuto_) {
      maxYValue_=tmpMax;
   }
   if (minYValueAuto_) {
      minYValue_=tmpMin;
   }

   curBuff_=(curBuff_+1)%numOfActivatedBuffers_;
   if (curBuff_ >= numOfActiveBuffers_) {
      numOfActiveBuffers_=curBuff_+1;
      bufferFill_->setProgress(numOfActiveBuffers_);
   }
   newIntegrationBuff_=true;
   newFrameAvaible();
   emit(numOfBufferChange(numOfActivatedBuffers_));
   emit(maxYValueChange(maxYValue_));
   emit(minYValueChange(minYValue_));
}

void QCamAdd::setNumOfBuffer(int nbuf) {

   if (nbuf<=0) {
      numOfActivatedBuffers_=numOfBuffers_/2;
   } else {
      numOfActivatedBuffers_=nbuf;
   }

   resetBufferFill();
   remoteCTRLmaxYvalue_->setMaxValue(numOfActivatedBuffers_*255);
   remoteCTRLminYvalue_->setMaxValue(numOfActivatedBuffers_*255);
}

void QCamAdd::setMaxYvalue(int val) {
   if (val <= 0) {
      maxYValueAuto_=true;
   } else {
      maxYValueAuto_=false;
      maxYValue_=val;
   }
   if (maxYValue_ <= minYValue_) {
      maxYValue_=minYValue_+1;
   }
}

void QCamAdd::setMinYvalue(int val) {
   if (val <= 0) {
      minYValueAuto_=true;
   } else {
      minYValueAuto_=false;
      minYValue_=val;
   }
   if (minYValue_ >= maxYValue_) {
      minYValue_=maxYValue_-1;
   }
}

QWidget * QCamAdd::buildGUI(QWidget * parent) {
   QWidget * remoteCTRL=QCam::buildGUI(parent);

   accumulationWidget_ = new QHGroupBox(tr("Num of Buffers"),remoteCTRL);
   int ActiveBufferList[]={1,2,4,8,16,32,64,128,256,512};
   remoteCTRLnumOfActiveBuffer_=
      new QCamComboBox(tr("Num of Buffers"),
                       accumulationWidget_,
                       10,ActiveBufferList,NULL);
   connect(this,SIGNAL(numOfBufferChange(int)),
           remoteCTRLnumOfActiveBuffer_,SLOT(update(int)));
   connect(remoteCTRLnumOfActiveBuffer_,SIGNAL(change(int)),
           this,SLOT(setNumOfBuffer(int)));

   bufferFill_= new QProgressBar(accumulationWidget_);
   bufferFill_->setCenterIndicator(true);
   resetBufferFill_= new QPushButton(tr("reset"),accumulationWidget_);
   connect(resetBufferFill_,SIGNAL(pressed()),this,SLOT(resetBufferFill()));

   displayOptions_=new QVGroupBox(tr("Display Options"),remoteCTRL);
   remoteCTRLmaxYvalue_=new QCamSlider(tr("max Lum."),true,displayOptions_,
                                       2,numOfBuffers_*255);
   remoteCTRLminYvalue_=new QCamSlider(tr("min Lum."),true,displayOptions_,
                                       1,numOfBuffers_*255-1);
   int valueList[]={0,1,2,3,4,5,6,7};
   const char * labelList[]={"none","log10","log","sqrt","^2","^3","^4","^5"};
   modeDisplayButton_= new QCamRadioBox(tr("Lum. conversion"),displayOptions_,
                                       8,valueList,labelList,4);
   connect(modeDisplayButton_,SIGNAL(change(int)),this,SLOT(modeDisplay(int)));
   modeDisplay(0);
   modeDisplayButton_->update(0);

   invDisplayButton_ = new QCheckBox(tr("negate"),modeDisplayButton_);
   connect(invDisplayButton_,SIGNAL(toggled(bool)),this,SLOT(negateDisplay(bool)));
#ifdef MultiSatMode
   int saturationValueList[]={0,1,2,3,4};
   const char * saturationLabelList[]={"none","low","norm","high","max"};
   maxCrSaturatedButton_= new QCamRadioBox(tr("Color saturation"),displayOptions_,
                                       5,saturationValueList,saturationLabelList,5);
   connect(maxCrSaturatedButton_,SIGNAL(change(int)),this,SLOT(maxSaturatedColors(int)));
   maxSaturatedColors(2);
   maxCrSaturatedButton_->update(2);
#else
   maxCrSaturatedButton_ = new QCheckBox(tr("Saturated colors"),displayOptions_);
   connect(maxCrSaturatedButton_,SIGNAL(toggled(bool)),this,SLOT(maxSaturatedColors(bool)));
#endif
   remoteCTRLmaxYvalue_->show();
   remoteCTRLminYvalue_->show();
   modeDisplayButton_->show();
   invDisplayButton_->show();
   displayOptions_->show();

   connect(this,SIGNAL(maxYValueChange(int)),
           remoteCTRLmaxYvalue_,SLOT(setValue(int)));
   connect(remoteCTRLmaxYvalue_,SIGNAL(valueChange(int)),
           this,SLOT(setMaxYvalue(int)));

   connect(this,SIGNAL(minYValueChange(int)),
           remoteCTRLminYvalue_,SLOT(setValue(int)));
   connect(remoteCTRLminYvalue_,SIGNAL(valueChange(int)),
           this,SLOT(setMinYvalue(int)));

   remoteCTRLnumOfActiveBuffer_->show();
   bufferFill_->show();
   accumulationWidget_->show();
   remoteCTRLmaxYvalue_->show();
   remoteCTRLminYvalue_->show();

   //remoteCTRL->setCaption("accumulation");
   return remoteCTRL;
}

void QCamAdd::resetBufferFill() {
   curBuff_=0;
   numOfActiveBuffers_=1;
   bufferFill_->setTotalSteps(numOfActivatedBuffers_);
   bufferFill_->reset();
   zeroBuff(size());
}

static double puis4(double val) {
   return pow(val,4);
}

static double puis5(double val) {
   return pow(val,5);
}

static double puis3(double val) {
   return pow(val,3);
}

static double puis2(double val) {
   return val*val;
}

void QCamAdd::modeDisplay(int val) {
   double (*newFuncDisplay_)(double);
   switch (val) {
   case 0: newFuncDisplay_=NULL; break;
   case 1: newFuncDisplay_=log10; break;
   case 2: newFuncDisplay_=log; break;
   case 3: newFuncDisplay_=sqrt; break;
   case 4: newFuncDisplay_=puis2; break;
   case 5: newFuncDisplay_=puis3; break;
   case 6: newFuncDisplay_=puis4; break;
   case 7: newFuncDisplay_=puis5; break;
   }
   if (newFuncDisplay_ != funcDisplay_) {
      funcDisplay_=newFuncDisplay_;
      newIntegrationBuff_=true;
      newFrameAvaible();
      //emit (newFrame());
   }
}
