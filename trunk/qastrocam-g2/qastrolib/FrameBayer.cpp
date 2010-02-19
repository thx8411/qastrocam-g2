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

bool FrameBayer::transform(const QCamFrame in, QCamFrame & out) {
   if (in.empty()) {
      return false;
   }

   // temp
   out=in;

   return true;
}

FrameBayer::FrameBayer(QCamTrans* cam) {
   cam_=cam;
}

FrameBayer::Widget::Widget(QWidget * parent,const FrameBayer * algo): QHBox(parent) {
   label1=new QLabel("Bayer pattern :",parent);
   int patternsValues[]={0,1,2,3,4};
   const char* patternsLabels[]={"none","GR/BG","RG/GB","BG/GR","GB/RG"};
   pattern = new QCamComboBox("Pattern",parent,5,patternsValues,patternsLabels);
   label2=new QLabel("Algorithm :",parent);
   int algoValues[]={0};
   const char* algoLabels[]={"Bilinear"};
   algorithm=new QCamComboBox("Pattern",parent,1,algoValues,algoLabels);
}

FrameBayer::Widget::~Widget() {
}
