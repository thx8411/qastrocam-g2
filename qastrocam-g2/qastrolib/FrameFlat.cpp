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

#include <qhbox.h>
#include <qimage.h>
#include <qmessagebox.h>

#include "yuv.hpp"

#include "FrameFlat.moc"

bool FrameFlat::transform(const QCamFrame in, QCamFrame & out) {
   if (in.empty()) {
      return false;
   }

   if(activated) {
      // compare frames (size, mode)
      if((flatFrame.getMode()!=in.getMode())||(flatFrame.size()!=in.size())) {
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
      out.applyFlat(flatFrame);
   } else
      out=in;

   return true;
}

FrameFlat::FrameFlat(QCamTrans* cam) {
   cam_=cam;
   activated=false;
}

FrameFlat::Widget::Widget(QWidget * parent,const FrameFlat * algo): QHBox(parent) {
   padding1=new QWidget(this);
   activate=new QCheckBox("Activate",this);
   padding2=new QWidget(this);
   label1=new QLabel("File : ",this);
   fileEntry=new QLineEdit(this);
   fileEntry->setDisabled(true);
   fileChooser=new QFileChooser(this,IMAGE_FILE);
   padding3=new QWidget(this);
   connect(activate,SIGNAL(stateChanged(int)),algo,SLOT(activatedChange(int)));
   connect(fileChooser,SIGNAL(fileChanged(const QString &)),algo,SLOT(fileChanged(const QString &)));
   connect(fileChooser,SIGNAL(fileChanged(const QString &)),fileEntry,SLOT(setText(const QString &)));
   connect(algo,SIGNAL(desactivated(bool)),activate,SLOT(setChecked(bool)));
}

FrameFlat::Widget::~Widget() {
}

void FrameFlat::activatedChange(int s) {
   if(s==QButton::On) {
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
            flatFrame.setMode(GreyFrame);
            flatFrame.setSize(QSize(width,height));
            YBuf=(unsigned char*)flatFrame.YforOverwrite();
            memcpy(YBuf,image.bits(),width*height);
            break;
         case 32 :
            flatFrame.setMode(YuvFrame);
            flatFrame.setSize(QSize(width,height));
            YBuf=(unsigned char*)flatFrame.YforOverwrite();
            UBuf=(unsigned char*)flatFrame.UforOverwrite();
            VBuf=(unsigned char*)flatFrame.VforOverwrite();
            bgr32_to_yuv444(width,height,image.bits(),YBuf,UBuf,VBuf);
            break;
         default :
            QMessageBox::information(0,"Qastrocam-g2","Frame depth not supported");
            emit(desactivated(false));
            return;
      }

      activated=true;
   } else
      activated=false;
}

void FrameFlat::fileChanged(const QString & name) {
   fileName=name;
   emit(desactivated(false));
}
