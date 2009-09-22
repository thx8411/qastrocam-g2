#ifndef DIP
#define DIP

#define IMAGE_WIDTH      640
#define IMAGE_HEIGHT     480
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

#define WORD short

#include <string>
#include <map>

using namespace std;

class QCamFrame;

class FitsImage {
protected:
   const string fileName_;
public:
   FitsImage(const string & fileName): fileName_(fileName) {}
   virtual bool load(QCamFrame &)=0;
   virtual bool save(const QCamFrame &)=0;
   virtual ~FitsImage() {}
   virtual bool close() {}
};

#if 0
class FitsImageFM: public FitsImage {
   WORD *f_;
   int rx_;
   int ry_;
   int lo_;
   int hi_;
   static int debug;
   map<string,string> externalProperties_;
public:
   FitsImageFM(const string & fileName);
   ~FitsImageFM();
   bool load(QCamFrame &);
   bool save(const QCamFrame &);
};

#endif


#include <fitsio.h>

class FitsImageCFITSIO : public FitsImage {
   fitsfile *fptr_;
   bool multipleFrame_;
   int status_;
   int numAxes_;
   long axesDim_[3];
   long base_[3];
private:
   bool initFits(const QCamFrame& frame);
public:
   FitsImageCFITSIO(const string & fileName,bool multipleFrame=false);
   ~FitsImageCFITSIO();
   bool load(QCamFrame &);
   bool save(const QCamFrame &);
   virtual bool close();
};

#endif /*DIP */
