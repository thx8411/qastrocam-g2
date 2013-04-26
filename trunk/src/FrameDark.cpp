/******************************************************************
Qastrocam-g2
Copyright (C) 2009-2013   Blaise-Florentin Collin

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

#include <Qt/qimage.h>
#include <Qt/qmessagebox.h>
#include <Qt/qtooltip.h>
#include <Qt/qlabel.h>

#include "yuv.hpp"
#include "FrameDark.hpp"

bool FrameDark::transform(const QCamFrame in, QCamFrame & out) {
   if (in.empty()) {
      return false;
   }

   if(activated) {
      // compare frames (size, mode)
      if((darkFrame.getMode()!=in.getMode())||(darkFrame.size()!=in.size())) {
         emit(desactivated(false));
         activated=false;
         QMessageBox::information(0,"Qastrocam-g2","Wrong frame size or colordepth");
         return(false);
      }
      // substract frame
      out.setMode(in.getMode());
      out.setSize(in.size());
      out.copy(in,
               0,0,
               in.size().width()-1,in.size().height()-1,
               0,0,
               false,false);
      out.applyDark(darkFrame,timeFactor);
   } else
      out=in;

   return true;
}

FrameDark::FrameDark(QCamTrans* cam) {
   cam_=cam;
   activated=false;
   timeFactor=1.0;
   timeString="1.0";
}

FrameDark::Widget::Widget(QWidget * parent,const FrameDark * algo): QCamHBox(parent) {
   padding1=new QWidget(this);
   activate=new QCheckBox("Activate",this);
   padding2=new QWidget(this);
   label1=new QLabel("File : ",this);
   fileEntry=new QLineEdit(this);
   fileEntry->setReadOnly(true);
   fileChooser=new QFileChooser(this,IMAGE_FILE);
   padding3=new QWidget(this);
   label2=new QLabel("Time factor :",this);
   timeEntry=new QLineEdit(this);
   timeEntry->setMaximumWidth(40);
   timeEntry->setText("1.0");
   padding4=new QWidget(this);
   connect(activate,SIGNAL(stateChanged(int)),algo,SLOT(activatedChange(int)));
   connect(fileChooser,SIGNAL(fileChanged(const QString &)),algo,SLOT(fileChanged(const QString &)));
   connect(fileChooser,SIGNAL(fileChanged(const QString &)),fileEntry,SLOT(setText(const QString &)));
   connect(timeEntry,SIGNAL(textChanged(const QString &)),algo,SLOT(timeChanged(const QString&)));
   connect(algo,SIGNAL(desactivated(bool)),activate,SLOT(setChecked(bool)));
   QToolTip::add(activate,tr("Activate or not the 'dark substraction' filter"));
   QToolTip::add(fileEntry,tr("Picture file to use as 'dark' frame"));
   QToolTip::add(fileChooser,tr("Selects the file to use as 'dark' frame"));
   QToolTip::add(timeEntry,tr("Sets the time factor to apply to your 'dark' frame"));
}

FrameDark::Widget::~Widget() {
}

void FrameDark::activatedChange(int s) {
   if(s==QCheckBox::On) {
      int depth;
      int width;
      int height;
      bool res;
      // load file
      QImage image;
      res=image.load(fileName);
      if(!res) {
         QMessageBox::information(0,"Qastrocam-g2","Unable to open the dark frame");
         emit(desactivated(false));
         return;
      }

      // test picture depth
      // and copy into frame
      depth=image.depth();
      width=image.width();
      height=image.height();
      unsigned char * YBuf=NULL,*UBuf=NULL,*VBuf=NULL;

      switch(depth) {
         case 8 :
            darkFrame.setMode(GreyFrame);
            darkFrame.setSize(QSize(width,height));
            YBuf=(unsigned char*)darkFrame.YforOverwrite();
            memcpy(YBuf,image.bits(),width*height);
            break;
         case 32 :
            darkFrame.setMode(YuvFrame);
            darkFrame.setSize(QSize(width,height));
            YBuf=(unsigned char*)darkFrame.YforOverwrite();
            UBuf=(unsigned char*)darkFrame.UforOverwrite();
            VBuf=(unsigned char*)darkFrame.VforOverwrite();
            bgr32_to_yuv444(width,height,image.bits(),YBuf,UBuf,VBuf);
            break;
         default :
            QMessageBox::information(0,"Qastrocam-g2","Frame depth not supported");
            emit(desactivated(false));
            return;
      }

      // test time factor
      if (sscanf(timeString.latin1(),"%lf",&timeFactor)!=1) {
         QMessageBox::information(0,"Qastrocam-g2","Wrong time factor");
         emit(desactivated(false));
         return;
      }

      activated=true;
   } else
      activated=false;
}

void FrameDark::fileChanged(const QString & name) {
   fileName=name;
   emit(desactivated(false));
}

void FrameDark::timeChanged(const QString & timeF) {
   timeString=timeF;
   emit(desactivated(false));
}
