/******************************************************************
Qastrocam
Copyright (C) 2003-2009   Franck Sicard
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


#ifndef _QCAMMOVIE_HPP_
#define _QCAMMOVIE_HPP_

#include <qobject.h>
#include <string>
#include <stdio.h>

using namespace std;

class QCamFrame;
class QCam;
class QString;
class QWidget;

class QCamMovie {
public:
   QCamMovie() {};
   virtual ~QCamMovie() {};
   virtual QWidget * buildGUI(QWidget  * father) { return father;}
   bool open(const string & seqName, const QCam & cam);
   void close();
   bool add(const QCamFrame & newFrame, const QCam & cam);
   int getFrameNumber() const { return frameNumber_; }
protected:
   virtual bool openImpl(const string & seqName, const QCam & cam)=0;
   virtual void closeImpl()=0;
   virtual bool addImpl(const QCamFrame & newFrame, const QCam & cam)=0;
private:
   FILE * propFile_;
   int frameNumber_;
};

#endif
