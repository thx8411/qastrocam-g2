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


#include "QCamMovie.hpp"
#include <errno.h>
#include "QCam.hpp"

bool QCamMovie::open(const string & seqName, const QCam & cam) {
   propFile_=fopen((seqName+".properties").c_str(),"w");

   if (propFile_ == NULL) {
      perror((seqName+".properties").c_str());
      return false;
   }

   fprintf(propFile_,"Frame");
   const map<string,string> props=cam.getProperties();
   for (map<string,string>::const_iterator it=props.begin();
        it!=props.end();
        ++it) {
      fprintf(propFile_,"; %s",it->first.c_str());
   }
   fprintf(propFile_,"\n");
   frameNumber_=0;
   return openImpl(seqName,cam);
}
void QCamMovie::close() {
   closeImpl();
   fclose(propFile_);
   propFile_=NULL;
   frameNumber_=0;
}

bool QCamMovie::add(const QCamFrame & newFrame, const QCam & cam) {
   const map<string,string> props=cam.getProperties();
   ++frameNumber_;
   fprintf(propFile_,"%d",frameNumber_);
   for (map<string,string>::const_iterator it=props.begin();
        it!=props.end();
        ++it) {
      fprintf(propFile_,"; %s",it->second.c_str());
   }
   fprintf(propFile_,"\n");
   return addImpl(newFrame,cam);
}
