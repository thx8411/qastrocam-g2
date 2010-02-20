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


#include "FrameDark.moc"
#include <qhbox.h>

bool FrameDark::transform(const QCamFrame in, QCamFrame & out) {
   if (in.empty()) {
      return false;
   }

   if(activated) {
      // substract frame
   } else
      out=in;

   return true;
}

FrameDark::FrameDark(QCamTrans* cam) {
   cam_=cam;
   activated=false;
}

FrameDark::Widget::Widget(QWidget * parent,const FrameDark * algo): QHBox(parent) {
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
}

FrameDark::Widget::~Widget() {
}

void FrameDark::activatedChange(int s) {
   if(s==QButton::On) {
      activated=true;

      // load file

      // compare frames (size, mode)
   } else
      activated=false;

}

void FrameDark::fileChanged(const QString & name) {
   fileName=name;
}
