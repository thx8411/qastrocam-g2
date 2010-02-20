/******************************************************************
Qastrocam-g2
Copyright (C) 2009   Blaise-Florentin Collin

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


#include "FrameBayer.moc"
#include <qhbox.h>

#include "SettingsBackup.hpp"

// settings object, needed everywhere
extern settingsBackup settings;

const char* patternsLabels[]={"none","GR/BG","RG/GB","BG/GR","GB/RG"};
const char* algoLabels[]={"Nearest","Bilinear"};

bool FrameBayer::transform(const QCamFrame in, QCamFrame & out) {
   ImageMode rawMode;
   DebayerMethod rawMethod;

   if (in.empty()) {
      return false;
   }

   // select bayer pattern
   switch(patternId) {
      case 0 : rawMode=(ImageMode)0; break;
      case 1 : rawMode=RawRgbFrame1; break;
      case 2 : rawMode=RawRgbFrame2; break;
      case 3 : rawMode=RawRgbFrame3; break;
      case 4 : rawMode=RawRgbFrame4; break;
   }
   // select debayer method
   switch(methodId) {
      case 0 : rawMethod=Nearest; break;
      case 1 : rawMethod=Bilinear; break;
   }
   // if debayer, copy the frame
   if(rawMode) {
      out.setMode(in.getMode());
      out.setSize(in.size());
      out.copy(in,
               0,0,
               in.size().width()-1,in.size().height()-1,
               0,0,
               false,false);
      out.debayer(rawMode,rawMethod);
   } else
   // else, simple refence
      out=in;
   return true;
}

FrameBayer::FrameBayer(QCamTrans* cam) {
   cam_=cam;
   patternId=0;
   methodId=0;
}

FrameBayer::Widget::Widget(QWidget * parent,const FrameBayer * algo): QHBox(parent) {
   padding1=new QWidget(this);
   label1=new QLabel("Bayer pattern :",this);
   int patternsValues[]={0,1,2,3,4};
   pattern = new QCamComboBox("Pattern",this,5,patternsValues,patternsLabels);
   padding2=new QWidget(this);
   label2=new QLabel("Algorithm :",this);
   int algoValues[]={0,1};
   algorithm=new QCamComboBox("Pattern",this,2,algoValues,algoLabels);
   padding3=new QWidget(this);
   connect(pattern,SIGNAL(change(int)),algo,SLOT(patternChanged(int)));
   connect(algorithm,SIGNAL(change(int)),algo,SLOT(algorithmChanged(int)));
   string keyName("RAW_MODE");
   if(settings.haveKey(keyName.c_str())) {
        int index=pattern->getPosition(settings.getKey(keyName.c_str()));
        if (index!=-1) {
           pattern->update(index);
           pattern->updateSignal(index);
        }
   } else pattern->update(0);
   keyName="RAW_METHOD";
   if(settings.haveKey(keyName.c_str())) {
        int index=algorithm->getPosition(settings.getKey(keyName.c_str()));
        if (index!=-1) {
           algorithm->update(index);
           algorithm->updateSignal(index);
        }
   // else use default
   } else algorithm->update(0);
}

FrameBayer::Widget::~Widget() {
}

void FrameBayer::patternChanged(int num) {
   patternId=num;
   string keyName("RAW_MODE");
   settings.setKey(keyName.c_str(),patternsLabels[num]);
}

void FrameBayer::algorithmChanged(int num) {
   methodId=num;
   string keyName("RAW_METHOD");
   settings.setKey(keyName.c_str(),algoLabels[num]);
}
