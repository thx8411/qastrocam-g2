#ifndef _PPort_hpp_
#define _PPort_hpp_

#include <string>

using namespace std;

// ports types :

#define LP_TYPE		0
#define PPDEV_TYPE	1

#define PPORT_TABLE_SIZE	4

typedef struct {
   string name;
   int type;
   int fd;
   unsigned char data;
} pportEntry;

class PPort {
private :
   // assuming there is no more than PPORT_TABLE_SIZE ports used at the same time
   pportEntry pportTable[PPORT_TABLE_SIZE];
   int pportTableSize;
   static PPort* instance_;
   PPort();
   ~PPort();
   int pportFind(const char* name);
public:
   static PPort* instance();
   static void destroy();
   int getAccess(const char* device);
   bool setBit(int bit, bool value, int entry);
};
#endif
