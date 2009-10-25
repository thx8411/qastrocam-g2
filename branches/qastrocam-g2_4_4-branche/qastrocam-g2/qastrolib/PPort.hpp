#ifndef _PPort_hpp_
#define _PPort_hpp_

#include <string>

using namespace std;

// ports types :
#define LP_TYPE		0
#define PPDEV_TYPE	1

#define PPORT_TABLE_SIZE	4

// opened port entry
typedef struct {
   // device name
   string name;
   // type : lp or parport
   int type;
   // file descriptor
   int fd;
   // bus byte value
   unsigned char data;
} pportEntry;

// parallel port access class, singleton
class PPort {
private :
   // assuming there is no more than PPORT_TABLE_SIZE ports used at the same time
   pportEntry pportTable[PPORT_TABLE_SIZE];
   int pportTableSize;
   // singleton class
   static PPort* instance_;
   PPort();
   ~PPort();
   // find port entry using his name, -1 if not found
   int pportFind(const char* name);
   // instance clients number
   static int clientNumber;
public:
   // singleton class
   static PPort* instance();
   static void destroy();
   // get access to a port, returns entry number, or -1 (error)
   int getAccess(const char* device);
   // change the port bit using entry number, true if success
   bool setBit(int bit, bool value, int entry);
};

#endif
