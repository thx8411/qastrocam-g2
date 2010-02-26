/******************************************************************
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
// TS_LEVELS_INVERTED
// SOURCE_<name>
// PALETTE
// TELESCOPE_DEVICE
// LX_DEVICE
// FORCE_V4LGENERIC

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
      // setting settings file name
      void setName(string name);
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