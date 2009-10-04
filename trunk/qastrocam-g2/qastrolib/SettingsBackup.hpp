#ifndef _SETTINGSBACKUP_H_
#define _SETTINGSBACKUP_H_

#include <string>

using namespace std;

#define NB_RECORDS	32

// store settings as a pair of strings : key<tab>value
// strings may contain spaces
//
// known and used keys :
//
// FILE_FORMAT
// FRAME_RESOLUTION
// RAW_MODE
// LX_LEVELS_INVERTED
// LX_LEVELS_INVERTED
// SOURCE_<name>
// PALETTE
// TELESCOPE_DEVICE
// LX_DEVICE

class settingsBackup {
   private :
      int end_record;
      string fileName;
      string datas[2][NB_RECORDS];
      int findKey(const char* key);
      void addKey(const char* key, const char* val);
      void serialize();
   public :
      settingsBackup();
      // checks if the key is in
      bool haveKey(const char* key);
      // set the key value, and create a key if it doesn't exist
      void setKey(const char* key, const char* val);
      // get the value from a known key,  NULL if unknown
      const char* getKey(const char* key);
      // load the settings file
      void deSerialize();
};

#endif
