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


#ifndef _SCmodParPortPPdev_hpp_
#define _SCmodParPortPPdev_hpp_

#include "SCmod.hpp"
#include "PPort.hpp"

#include <string>

using namespace std;

// driving lxmode using part port
class SCmodParPortPPdev : public SCmod {
public:
   SCmodParPortPPdev();
   virtual ~SCmodParPortPPdev();
   void enterLongPoseMode();
   void leaveLongPoseMode();
   void stopAccumulation();
   void startAccumulation();
private:
   // lists used par port bits
   enum pportBit {evenLinesTransfer=0,
                  oddLinesTransfer=1,
                  preamp=2,
                  shutter=3};
   // entry i PPdev table
   int portEntry;
   // PPdev object
   PPort* paralPort;
   // are logical level on the port inverted ?
   bool inverted;
   // port device name
   string device;
};

#endif
