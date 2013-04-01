/******************************************************************
Qastrocam
Copyright (C) 2003-2009   Franck Sicard
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


#ifndef _QTelescope_hpp_
#define _QTelescope_hpp_

#include <qobject.h>
#include <qslider.h>
#include <qlabel.h>
#include <qcheckbox.h>

// telescope types
#define TELESCOPE_VIRTUAL	0
#define TELESCOPE_APM		1
#define TELESCOPE_AUTOSTAR	2
#define TELESCOPE_FIFO		3
#define TELESCOPE_FILE		4
#define TELESCOPE_MCU		5
#define TELESCOPE_MTS		6
#define TELESCOPE_NEXSTAR	7
#define TELESCOPE_QHY5		8
#define TELESCOPE_LX200		9
#define TELESCOPE_SIMULATOR	10
#define TELESCOPE_QHY6		11

class QVGroupBox;
class QPushButton;
class QGridLayout;
class QWidget;

/** Base class to command a telescope.
 */
class QTelescope : public QObject {
   Q_OBJECT
public:
   QTelescope();
   /** build the gui of the QTelescope widget.
       if it is overloaded, parents buildGUI should be called
       by new version. */
   virtual void buildGUI(QWidget * parent=0);
   // update shifts when a new frame comes
   virtual void Update(double, double) {};
   // reset telescope when tracker reseted
   virtual void Reset() {};
   virtual void setTrack(bool) {};
   virtual ~QTelescope();
   virtual int telescopeType() { return(TELESCOPE_VIRTUAL); }
protected:
   QWidget * widget();
   double currentSpeed;
private:
   QVGroupBox * mainWidget_;
   QGridLayout * arrowsLayout_;
   QWidget * arrows_;
   // speed stuff
   QSlider* speedSlider_;
   QLabel* speedValue_;
   // move buttons
   QPushButton * upButton_;
   QPushButton * downButton_;
   QPushButton * leftButton_;
   QPushButton * rightButton_;
   // swap box
   QCheckBox* raSwap_;
   QCheckBox* decSwap_;
   /** copy constructor not implemented. */
   QTelescope(const QTelescope&);
private slots:
   void speedChanged(int speed);
public slots:
   // buttons slots
   void goUp();
   void goDown();
   void goLeft();
   void goRight();
   void stopUp();
   void stopDown();
   void stopLeft();
   void stopRight();
   // checkbox slots
   void swapRa(bool s);
   void swapDec(bool s);
   // move slots
   /// move East
   virtual void goE(float shift)=0;
   /// move West
   virtual void goW(float shift)=0;
   /// move South
   virtual void goS(float shift)=0;
   /// move North
   virtual void goN(float shift)=0;
   /// move East
   virtual void goE() { goE(0);}
   /// move West
   virtual void goW() { goW(0);}
   /// move South
   virtual void goS() { goS(0);}
   /// move North
   virtual void goN() { goN(0);}
   /// stop East move
   virtual void stopE()=0;
   /// stop South move
   virtual void stopS()=0;
   /// stop West move
   virtual void stopW()=0;
   /// stop North move
   virtual void stopN()=0;
   /**set Telescope move speed.
      0=> slowest speed, 1.0=> max speed.
      the speed entered is an hint. The function
      must return the real speed. 0 means 'not supported'
   */
   virtual double setSpeed(double speed)=0;
   /** set auto tracking mode.
    true=>tracking activated. false=>no tracking.
    the return value indicate if it was possible:
    a mount with no tracking mode will allways
    return !activated (it can always stop).*/
   virtual bool setTracking(bool activated)=0;
};

#endif
