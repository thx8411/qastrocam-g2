/******************************************************************
Qastrocam
Copyright (C) 2003-2009   Franck Sicard
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


#ifndef _QCamClient_hpp_
#define _QCamClient_hpp_

#include <Qt/qobject.h>

class QCam;

/** base class for objects that are using the frames
    generated by QCam objects.
    The slot newFrame() wich must be implemented will be
    called when a new frame is avable on the associated
    QCam and when the QCamClient object is not paused.
*/
class QCamClient : public QObject {
   Q_OBJECT
public:
   QCamClient();
   QCamClient(QCam & cam);
   virtual ~QCamClient() {};
   /** to connect a new camera. */
   void connectCam(QCam & cam);
   /** disconnect the current camera */
   void disconnectCam();
   /** stop the delivery of newFrame signal.*/
   void pause();
   /** resume the delivery of newFrame signal.*/
   void resume();
   /** predicat to know if a QCam object is connected */
   bool isConnected() const { return cam_ != NULL; }
   /** indicate if the client is active.
       if true, the clien is connected to a cam, and is not paused. */
   bool isActive() const { return isConnected() && !paused_;}
   /** return the connected QCam object.
       Must not be called if no QCam object is connected. */
   QCam & cam() const {return *cam_;}

   /** build the GUI related tgo the camera client.
       buildGUI of base class should also be called */
   QWidget * buildGUI(QWidget *parent);

   /** return the quick description of this client */
   virtual const QString & label() const { static QString aLabel="A Camera Client"; return aLabel;}
 public slots:
 /** wrapper slot for pause()/resume() */
 void disable(bool active);
protected:
   /** is called when a cam is connected. */
   virtual void camDisconnected(){};
   /** is called whne the current cam is deconnected. */
   virtual void camConnected(){};
protected slots:
   /** Slot to implement.
       is is called for every new frame.
       Use cam().yuvFrame() to acces the to frame.*/
   virtual void newFrame()=0;
private:
   QCam * cam_;
   bool paused_;
};
#endif
