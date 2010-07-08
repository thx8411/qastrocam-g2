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
#include <qradiobutton.h>
#include <qmessagebox.h>
#include <qtooltip.h>

#include <stdlib.h>
#include <math.h>

#include "QCamRadioBox.hpp"
#include "QCamComboBox.hpp"

const int QCamAdd::numOfBuffers_=128;

void QCamAdd::removeFrame(const QCamFrame & frame) {
   int dummyMin,dummyMax,dummyCr;
   moveFrame(frame,dummyMin,dummyMax,dummyCr,false);
}

void QCamAdd::removeAverageFrame(const QCamFrame & frame) {
   int i,ysize,usize;
   int* dst;
   const unsigned char* src=frame.Y();
   const unsigned char* usrc=frame.U();
   const unsigned char* vsrc=frame.V();

   // luminance
   dst=integrationBuff_;
   ysize=frame.ySize();
   for(i=0;i<ysize;i++) {
      dst[i]-=src[i];
   }
   // colors
   if(mode_==YuvFrame) {
      usize=frame.uSize();
      for(i=0;i<usize;i++) {
         dst[i+ysize]-=usrc[i];
         dst[i+ysize]+=128;
         dst[i+ysize+usize]-=vsrc[i];
         dst[i+ysize+usize]+=128;
      }
   }
}


void QCamAdd::removeMedianFrame(const QCamFrame & frame) {
   int i,ysize,usize;
   unsigned char* dst;
   const unsigned char* src=frame.Y();
   const unsigned char* usrc=frame.U();
   const unsigned char* vsrc=frame.V();

   dst=(unsigned char*)integrationBuff_;
   ysize=frame.ySize();

   // luminance
   for(i=0;i<ysize;i++) {
      dst[i*256+src[i]]--;
   }
   // colors
   if(mode_==YuvFrame) {
      usize=frame.uSize();
      for(i=0;i<usize;i++) {
         dst[(i+ysize)*256+usrc[i]]--;
         dst[(i+ysize+usize)*256+vsrc[i]]--;
      }
   }
}

void QCamAdd::addFrame(const QCamFrame & frame,int & maxYValue,int & minYValue,int & maxCrValue) {
   moveFrame(frame,maxYValue,minYValue,maxCrValue,true);
}

void QCamAdd::averageFrame(const QCamFrame & frame) {
   int i,ysize,usize;
   int* dst;
   const unsigned char* src=frame.Y();
   const unsigned char* usrc=frame.U();
   const unsigned char* vsrc=frame.V();

   // luminance
   dst=integrationBuff_;
   ysize=frame.ySize();
   for(i=0;i<ysize;i++) {
      dst[i]+=src[i];
   }
   // colors
   if(mode_==YuvFrame) {
      usize=frame.uSize();
      for(i=0;i<usize;i++) {
         dst[i+ysize]+=usrc[i]-128;
         dst[i+ysize+usize]+=vsrc[i]-128;
      }
   }
}

void QCamAdd::medianFrame(const QCamFrame & frame) {
   int i,ysize,usize;
   unsigned char* dst;
   const unsigned char* src=frame.Y();
   const unsigned char* usrc=frame.U();
   const unsigned char* vsrc=frame.V();

   dst=(unsigned char*)integrationBuff_;
   ysize=frame.ySize();

   // luminance
   for(i=0;i<ysize;i++) {
      dst[i*256+src[i]]++;
   }
   // colors
   if(mode_==YuvFrame) {
      usize=frame.uSize();
      for(i=0;i<usize;i++) {
         dst[(i+ysize)*256+usrc[i]]++;
         dst[(i+ysize+usize)*256+vsrc[i]]++;
      }
   }
}

void QCamAdd::moveFrame(const QCamFrame & frame,int & maxYValue,int & minYValue,int & maxCrValue,const bool adding) {
   int size;
   int tmpValue;
   int* dest=integrationBuff_;
   const uchar * src=frame.Y();
   maxYValue=minYValue=integrationBuff_[0];

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
   // color
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
            cout << "invalid value "<< maxCrValueAutoSaturated_ << "for maxCrValueAutoSaturated_\n";
      }
   } else {
      maxCrValue=maxYValue>>1;
   }
}

QCamAdd::QCamAdd(QCam* cam) :
   curSize_(0,0) {
   int nbuf=numOfBuffers_;
   method_=QCAM_ADD_ADD;
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
   delete [] frameHistory_;
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

void QCamAdd::average2yuv(const int * integration,QCamFrame & yuv) const {
   unsigned char* Ybuf;
   unsigned char* Ubuf;
   unsigned char* Vbuf;
   int i,ysize,usize,tmp;

   yuv.setMode(mode_);
   // luminance
   Ybuf=yuv.YforUpdate();
   ysize=yuv.ySize();
   for(i=0;i<ysize;i++) {
      if(numOfActiveBuffers_==0)
         tmp=255;
      else
         tmp=integration[i]/numOfActiveBuffers_;
      if(tmp<0)
         tmp=0;
      if(tmp>255)
         tmp=255;
      Ybuf[i]=tmp;
   }
   // colors
   if(mode_==YuvFrame) {
      usize=yuv.uSize();
      Ubuf=yuv.UforUpdate();
      Vbuf=yuv.VforUpdate();
      for(i=0;i<usize;i++) {
         if(numOfActiveBuffers_==0)
            tmp=255;
         else
            tmp=(integration[i+ysize]/numOfActiveBuffers_)+128;
         if(tmp<0)
            tmp=0;
         if(tmp>255)
            tmp=255;
         Ubuf[i]=tmp;
         if(numOfActiveBuffers_==0)
            tmp=255;
         else
            tmp=(integration[i+ysize+usize]/numOfActiveBuffers_)+128;
         if(tmp<0)
            tmp=0;
         if(tmp>255)
            tmp=255;
         Vbuf[i]=tmp;
      }
   }
}

void QCamAdd::median2yuv(const int * integration,QCamFrame & yuv) const {
   unsigned char* Ybuf;
   unsigned char* Ubuf;
   unsigned char* Vbuf;
   int i,ysize,usize,index;
   unsigned char value;
   unsigned char* src;

   src=(unsigned char*)integration;
   yuv.setMode(mode_);
   // luminance
   Ybuf=yuv.YforUpdate();
   ysize=yuv.ySize();
   for(i=0;i<ysize;i++) {
      value=0;
      index=0;
      while((value<(numOfActiveBuffers_/2))&&(index<256)) {
         value+=src[i*256+index];
         index++;
      }
      Ybuf[i]=index;
   }
   // colors
   if(mode_==YuvFrame) {
      Ubuf=yuv.YforUpdate();
      Vbuf=yuv.YforUpdate();
      usize=yuv.uSize();
      for(i=0;i<usize;i++) {
         value=0;
         index=0;
         while((value<(numOfActiveBuffers_/2))&&(index<256)) {
            value+=src[(i+ysize)*256+index];
            index++;
         }
         Ubuf[i]=index;
         value=0;
         index=0;
         while((value<(numOfActiveBuffers_/2))&&(index<256)) {
            value+=src[(i+ysize+usize)*256+index];
            index++;
         }
         Vbuf[i]=index;
      }
   }
}

QCamFrame QCamAdd::yuvFrame() const {
   if (newIntegrationBuff_) {
      switch(method_) {
         case QCAM_ADD_ADD :
            integration2yuv(integrationBuff_,computedFrame_);
            break;
         case QCAM_ADD_AVERAGE :
            average2yuv(integrationBuff_,computedFrame_);
            break;
         case QCAM_ADD_MEDIAN :
            median2yuv(integrationBuff_,computedFrame_);
            break;
      }
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
   if(method_==QCAM_ADD_MEDIAN) {
      memset(integrationBuff_,0,size.height()*size.width()*256*3);
   } else {
      memset(integrationBuff_,0,sizeof(int) * size.height()*size.width()*3);
   }
   for (int i=0;i<numOfActivatedBuffers_;++i) {
      frameHistory_[i].clear();
   }
}

void QCamAdd::allocBuff(const QSize & size) {
   free(integrationBuff_);
   if(method_==QCAM_ADD_MEDIAN) {
      integrationBuff_=(int*)malloc(size.height()*size.width()*256*3);
   } else {
      integrationBuff_=(int*)malloc(size.height()*size.width()*3*sizeof(int));
   }
   if(integrationBuff_==NULL) {
      QMessageBox::information(0,"Qastrocam-g2","Unable allocate the filter memory, leaving...");
      exit(1);
   }
   computedFrame_.setSize(size);

   for (int i=0;i<numOfBuffers_;++i) {
      frameHistory_[i].setSize(size);
   }

   zeroBuff(size);
}

void QCamAdd::addFrame(const QCamFrame & frame) {
   if ((curSize_ != cam_->size())||(mode_!= frame.getMode())) {
      allocBuff( cam_->size());
      curSize_= cam_->size();
      mode_=frame.getMode();
      resetBufferFill();
   }

   switch(method_) {
      // add a frame
      case QCAM_ADD_ADD :
         int tmpMax,tmpMin;
         switch (frame.getMode()) {
            case GreyFrame:
               mode_=GreyFrame;
               if(maxCrSaturatedButton_)
                  maxCrSaturatedButton_->hide();
               break;
            case YuvFrame:
               mode_=(maxCrValueAutoSaturated_==0)?GreyFrame:YuvFrame;
               if(maxCrSaturatedButton_)
                  maxCrSaturatedButton_->show();
               break;
         }
         removeFrame(frameHistory_[curBuff_]);
         frameHistory_[curBuff_]=frame;

         if (maxYValueAuto_) {
            maxYValue_=-256*256;
         }
         if (minYValueAuto_) {
            minYValue_=256*256;
         }
         addFrame(frameHistory_[curBuff_],tmpMax,tmpMin,maxCrValue_);
         if (maxYValueAuto_) {
            maxYValue_=tmpMax;
         }
         if (minYValueAuto_) {
            minYValue_=tmpMin;
         }
            emit(numOfBufferChange(numOfActivatedBuffers_));
            emit(maxYValueChange(maxYValue_));
            emit(minYValueChange(minYValue_));
         break;
      // average frame
      case QCAM_ADD_AVERAGE :
         //
         removeAverageFrame(frameHistory_[curBuff_]);
         frameHistory_[curBuff_]=frame;
         averageFrame(frameHistory_[curBuff_]);
         //
         break;
      case QCAM_ADD_MEDIAN :
         //
         removeMedianFrame(frameHistory_[curBuff_]);
         frameHistory_[curBuff_]=frame;
         medianFrame(frameHistory_[curBuff_]);
         //
         break;
   }

   curBuff_=(curBuff_+1)%numOfActivatedBuffers_;
   if (curBuff_ >= numOfActiveBuffers_) {
      numOfActiveBuffers_=curBuff_+1;
      bufferFill_->setProgress(numOfActiveBuffers_);
   }
   newIntegrationBuff_=true;
   newFrameAvaible();
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

   // method gui
   methodWidget_ = new QButtonGroup(3,Qt::Horizontal,tr("Method"), remoteCTRL);
   QRadioButton* frameSum= new QRadioButton(tr("Sum"),methodWidget_);
   QRadioButton* frameAverage= new QRadioButton(tr("Average"),methodWidget_);
   QRadioButton* frameMedian= new QRadioButton(tr("Median"),methodWidget_);
   methodWidget_->setButton(0);
   methodWidget_->setMaximumHeight(52);
   connect(methodWidget_,SIGNAL(clicked(int)),this,SLOT(methodChanged(int)));

   QToolTip::add(frameSum,tr("Adds the frames in live"));
   QToolTip::add(frameAverage,tr("Produce a 'mean' frame for calibration"));
   QToolTip::add(frameMedian,tr("Produce a 'median' frame for calibration\n(uses a huge amount of memory)"));

   accumulationWidget_ = new QHGroupBox(tr("Num of Buffers"),remoteCTRL);
   accumulationWidget_->setMaximumHeight(56);
   int ActiveBufferList[]={1,2,4,8,16,32,64,128};
   remoteCTRLnumOfActiveBuffer_=new QCamComboBox(tr("Num of Buffers"),accumulationWidget_,8,ActiveBufferList,NULL);
   connect(this,SIGNAL(numOfBufferChange(int)),remoteCTRLnumOfActiveBuffer_,SLOT(update(int)));
   connect(remoteCTRLnumOfActiveBuffer_,SIGNAL(change(int)),this,SLOT(setNumOfBuffer(int)));

   QToolTip::add(remoteCTRLnumOfActiveBuffer_,tr("Number of frames to stack"));

   bufferFill_= new QProgressBar(accumulationWidget_);
   bufferFill_->setCenterIndicator(true);
   resetBufferFill_= new QPushButton(tr("reset"),accumulationWidget_);
   connect(resetBufferFill_,SIGNAL(pressed()),this,SLOT(resetBufferFill()));

   QToolTip::add(bufferFill_,tr("Frame stack progress"));
   QToolTip::add(resetBufferFill_,tr("Resets the frame stack"));

   displayOptions_=new QVGroupBox(tr("Display Options"),remoteCTRL);
   displayOptions_->setMaximumHeight(192);
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

   QToolTip::add(displayOptions_,tr("Resulting frame tuning"));

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

   //remoteCTRLnumOfActiveBuffer_->setCurrentItem(4);

   remoteCTRLnumOfActiveBuffer_->show();
   bufferFill_->show();
   accumulationWidget_->show();
   accumulationWidget_->resize(accumulationWidget_->minimumSizeHint());
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

void QCamAdd::methodChanged(int b) {
   method_=b;
   allocBuff(cam_->size());
   resetBufferFill();
   if(method_!=0)
      displayOptions_->setEnabled(false);
   else
      displayOptions_->setEnabled(true);
}
