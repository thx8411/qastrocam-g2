/******************************************************************
Qastrocam-g2
Copyright (C) 2009-2010   Blaise-Florentin Collin

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

#define NB_RECORDS	64

#define _CONF_VERSION_  "2"

// store settings as a pair of strings : key<tab>value
// strings may contain spaces
//
// known and used keys :
//
// configuration file version :
//
// CONF_VERSION <2>	(conf version 2)
//
// device dependent :
//
// SOURCE_* <name>
// PALETTE_* <rgb24/yuyv/yuv420/grey/BA81>
// COLOR_MODE_* <Grey/Color>
//
// global :
//
// FILE_FORMAT <FITS/AVI raw/AVI huff/SER/BMP/PNG>
// FRAME_RESOLUTION <width>x<height>
// FRAME_MODE <scaling/cropping/binning>
// LX_LEVELS_INVERTED <yes/no>
// LX_DEVICE <file>
// FORCE_V4LGENERIC <yes/no>
// SDL <yes/no>
// EXPERT <yes/no>
// LOG <yes/no>
// REGISTAX_AVI <yes/no>
// NIGHT_VISION <yes/no>
// TELESCOPE <none/qhy5/qhy6/apm/autostar/nexstar/fifo/mcu/mts/file/lx200/simulator>	(qhy6 in conf version 2 only)
// TS_LEVELS_INVERTED <yes/no>
// TELESCOPE_DEVICE <file>
// TELESCOPE_SPEED <0 to 1>
// CAMERA <v4l(2)/qhy5/qhy6/simulator>	(conf version 2)
// CAMERA_DEVICE <file>	(conf version 2)
// VIDEO_DEVICE <file>	(conf version 1, outdated, use CAMERA_DEVICE instead)
// LIB_PATH <directory>
// ADD_MODULE <yes/no>
// MAX_MODULE <yes/no>
// MIRROR_MODULE <yes/no>
// KING_MODULE <yes/no>
// ALIGN_MODULE <yes/no>
// RAW_MODE <none/ GR/BG / RG/GB / BG/GR / GB/RG >
// RAW_METHOD <Nearest/Bilinear>
//
// QHY5 dedicated :
//
// QHY5_GAIN_G1 <value>
// QHY5_GAIN_G2 <value>
// QHY5_GAIN_R <value>
// QHY5_GAIN_B <value>
// QHY5_EXPOSURE <time in ms>
// QHY5_BANDS_FILTER <yes/no>	(conf version 2)
//
// QHY6 dedicated :	(conf version 2)
//
// QHY6_GAIN <value>
// QHY6_EXPOSURE <time in ms>
//
// GUIDING :
//
// GUIDE_RA_INVERSION <yes/no>
// GUIDE_DEC_INVERSION <yes/no>
// GUIDE_CENTER <yes/no>
// GUIDE_MIN_RA <value>
// GUIDE_MAX_RA <value>
// GUIDE_MIN_DEC <value>
// GUIDE_MAX_DEC <value>
// GUIDE_ALERT <yes/no>			(conf version 2)
//
// TELESCOPE :
//
// TELESCOPE_RA_INVERSION <yes/no>	(conf version 2)
// TELESCOPE_DEC_INVERSION <yes/no>	(conf version 2)
//
// GUI :
//
// GUI_NAS_MESSAGE <yes/no>		(conf version 2)
//

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
      // load the settings file, true on success
      bool deSerialize();
};

#endif
