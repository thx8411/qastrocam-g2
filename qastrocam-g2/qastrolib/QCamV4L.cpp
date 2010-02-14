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


#include "QCamV4L.moc"
#include <iostream>
#include <sstream>

#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <sys/mman.h>
#include <string>
#include <qtabwidget.h>
#include <qsocketnotifier.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qmessagebox.h>

#include "QCamSlider.hpp"
#include "yuv.hpp"
#include "QGridBox.hpp"
#include "QCamComboBox.hpp"
#include "SettingsBackup.hpp"

#include "SCmodParPortPPdev.hpp"

// settings object, needed everywhere
extern settingsBackup settings;

// static value init
const int QCamV4L::DefaultOptions=(haveBrightness|haveContrast|haveHue|haveColor|haveWhiteness);

// constructor
QCamV4L::QCamV4L(const char * devpath, unsigned long options /* cf QCamV4L::options */) {
}

// resize the stream
void QCamV4L::resize(const QSize & s) {
   setSize(s.width(),s.height());
}

void QCamV4L::updatePalette() {
}

// allocate memory for buffers, depending on
// frame size and palette
void QCamV4L::allocBuffers() {
}

// get frame sizes supported by the
// video device
const QSize * QCamV4L::getAllowedSize() const {
   return sizeTable_;
}

// change the frame size
bool QCamV4L::setSize(int x, int y) {
   return(true);
}

// drop frames without treatment
bool QCamV4L::dropFrame() {
   return(true);
}

// we should have a new frame
bool QCamV4L::updateFrame() {
   return(true);
}

const QSize & QCamV4L::size() const {
   return outputBuffer_.size();
}

void QCamV4L::setContrast(int val) {
}

int QCamV4L::getContrast() const {
   return(0);
}

void QCamV4L::setBrightness(int val) {
}

int QCamV4L::getBrightness() const {
   return(0);
}

void QCamV4L::setColor(int val) {
}

int QCamV4L::getColor() const {
   return(0);
}

void QCamV4L::setHue(int val) {
}

int QCamV4L::getHue() const {
   return(0);
}

void QCamV4L::setWhiteness(int val) {
}

int QCamV4L::getWhiteness() const {
   return(0);
}

QCamV4L::~QCamV4L() {
}

void QCamV4L::updatePictureSettings() {
}

void QCamV4L::refreshPictureSettings() {
}

QWidget * QCamV4L::buildGUI(QWidget * parent) {
   QWidget * remoteCTRL=QCam::buildGUI(parent);
   return remoteCTRL;
}

void QCamV4L::setSource(int val) {
}

// changing palette
void QCamV4L::setPalette(int val) {
}

// changing raw mode
void  QCamV4L::setMode(int  val) {
}

// changing raw mode
void  QCamV4L::setMode(ImageMode val) {
}

// setting lx modes
void QCamV4L::setLXmode(int value) {
}

// changing integration time
void QCamV4L::setLXtime() {
}

// lx timer timeout slot, stops integration
void QCamV4L::LXframeReady() {
}

// mmap init
bool QCamV4L::mmapInit() {
   return(true);
}

// mmap capture
uchar* QCamV4L::mmapCapture() {
   return(NULL);
}

void QCamV4L::mmapRelease() {
}

// gives os time in second (usec accuracy)
double QCamV4L::getTime() {
   double t;
   struct timeval tv;
   gettimeofday(&tv,NULL);
   t=(float)tv.tv_usec/(float)1000000;
   t+=tv.tv_sec;
   return(t);
}
