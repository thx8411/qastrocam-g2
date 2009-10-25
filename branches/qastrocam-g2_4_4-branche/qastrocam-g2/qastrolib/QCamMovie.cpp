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
