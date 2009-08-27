#include <list>
#include <string>

using namespace std;

class Fits {
public:
   enum dataType {notDefined,notSigned8bit, signed16bit,signed32bit}; 
   Fits();
   ~Fits();
   void connectData(int width, int height,
                    dataType type, const void * data);
   void setProperty(string name,string value);
   void setProperty(string name,bool value);
   void setProperty(string name,int value);
   void setProperty(string name,double value);
   bool write(const string & fileName) const;
private:
   static string padTo8(string);
   static string padTo80(string);
   /// width of image;
   int x_;
   /// hight of image;
   int y_;
   /// bit per pixel of image (8/16)
   dataType dataType_;
   const void * data_;
   list<string> properties_;
};
