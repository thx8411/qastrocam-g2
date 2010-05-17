/******************************************************************
Qastrocam-g2
Copyright (C) 2010 Blaise-Florentin Collin

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

// QHY5 native driver access
// singleton class

#include <stdlib.h>

#include "QHY5cam.hpp"

//
// singleton stuff
//

// static members init
QHY5cam* QHY5cam::instance_=NULL;
bool QHY5cam::feature_used[2]={false,false};

QHY5cam* QHY5cam::instance(int feature) {
   if(feature>=2)
      return(NULL);
   if(instance_==NULL) {
      instance_=new QHY5cam();
      feature_used[feature]=true;
      return(instance_);
   }
   if(feature_used[feature])
      return(NULL);
   return(instance_);
}

void QHY5cam::destroy(int feature) {
   feature_used[feature]=false;
   if(!(feature_used[QHY_IMAGER] || feature_used[QHY_GUIDER])) {
      delete instance_;
      instance_=NULL;
   }
}

//
// functions
//

QHY5cam::QHY5cam() {
}

QHY5cam::~QHY5cam() {
}

