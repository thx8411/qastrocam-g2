#include "QTelescope.moc"

#include <qlayout.h>
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qpushbutton.h>

#include "QCamUtilities.hpp"

QTelescope::QTelescope() {
   mainWidget_=NULL;
   arrowsLayout_=NULL;
   arrows_=NULL;
   upButton_=NULL;
   downButton_=NULL;
   leftButton_=NULL;
   rightButton_=NULL;
}

void QTelescope::buildGUI(QWidget * parent) {
   mainWidget_=new QVGroupBox("Telescope CTRL",parent);
   QCamUtilities::setQastrocamIcon(mainWidget_);
   arrows_ = new QWidget(mainWidget_);
   arrowsLayout_=new QGridLayout(arrows_,3,3);
   upButton_=new QPushButton(arrows_,"U");
   upButton_->setPixmap(*QCamUtilities::getIcon("up.png"));
   downButton_=new QPushButton(arrows_,"D");
   downButton_->setPixmap(*QCamUtilities::getIcon("down.png"));
   leftButton_=new QPushButton(arrows_,"L");
   leftButton_->setPixmap(*QCamUtilities::getIcon("left.png"));
   rightButton_=new QPushButton(arrows_,"R");
   rightButton_->setPixmap(*QCamUtilities::getIcon("right.png"));
   arrowsLayout_->addWidget(upButton_,0,1);
   arrowsLayout_->addWidget(downButton_,2,1);
   arrowsLayout_->addWidget(leftButton_,1,0);
   arrowsLayout_->addWidget(rightButton_,1,2);
   upButton_->show();
   arrows_->show();
   connect(upButton_,SIGNAL(pressed()),this,SLOT(goN()));
   connect(upButton_,SIGNAL(released()),this,SLOT(stopN()));
   connect(downButton_,SIGNAL(pressed()),this,SLOT(goS()));
   connect(downButton_,SIGNAL(released()),this,SLOT(stopS()));
   connect(leftButton_,SIGNAL(pressed()),this,SLOT(goE()));
   connect(leftButton_,SIGNAL(released()),this,SLOT(stopE()));
   connect(rightButton_,SIGNAL(pressed()),this,SLOT(goW()));
   connect(rightButton_,SIGNAL(released()),this,SLOT(stopW()));
   mainWidget_->show();
}

QTelescope::~QTelescope() {
   delete mainWidget_;
   delete arrowsLayout_;
   delete arrows_;
   delete upButton_;
   delete downButton_;
   delete leftButton_;
   delete rightButton_;
}

QWidget * QTelescope::widget() {
   return mainWidget_;
}
