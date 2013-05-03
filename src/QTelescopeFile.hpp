/******************************************************************
Qastrocam-g2
Copyright (C) 2009-2013   Blaise-Florentin Collin

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


#ifndef _QTelescopeFile_hpp_
#define _QTelescopeFile_hpp_

#include <string>

#include "QTelescope.hpp"

using namespace std;

/** Telescope implementation for a file interface.
 */
class QTelescopeFile : public QTelescope {
   Q_OBJECT
public:
   QTelescopeFile(const char* filePath);
   // gui
   virtual void buildGUI(QWidget* parent=0);
   // update shitfing value when a new frame comes
   virtual void Update(double x, double y);
   // creat new files when the tracker is reset
   virtual void Reset();
   // parent class compatibility
   virtual void setTrack(bool tracking);
   virtual int telescopeType() { return(TELESCOPE_FILE); }
public slots:
   // moves
   virtual void goE(float shift){};
   virtual void goW(float shift){};
   virtual void goS(float shift){};
   virtual void goN(float shift){};
   // stop
   virtual void stopW() {}
   virtual void stopE() {}
   virtual void stopN() {}
   virtual void stopS() {}
   // speed and track
   virtual double setSpeed(double speed) {return(0); }
   virtual bool setTracking(bool activated) {return true; }
  private:
   // timing
   double getTime();
   double sessionTime;
   void writeToFile();
   // files
   string genericFilePath;
   string filePathx;
   string filePathy;
   FILE* descriptorx_;
   FILE* descriptory_;
   // current datas
   float xPosition;
   float yPosition;
   bool tracking_;
};

#endif
