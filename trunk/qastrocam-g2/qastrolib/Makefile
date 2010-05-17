# Qastrocam
# Copyright (C) 2003-2009   Franck Sicard
# Qastrocam-g2
# Copyright (C) 2009   Blaise-Florentin Collin

# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License v2
# as published by the Free Software Foundation.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
# MA  02110-1301, USA.

include ../Makefile.vars

LIBNAME=qastro
TMP_DESTDIR=../lib

OTHERINC=-I/usr/include/SDL
OTHERLIB=

OBJ_QT = QCam.o QCamSlider.o QCamV4L.o QCamVesta.o QCamAdd.o \
         QCamRadioBox.o CamHistogram.o QCamClient.o QHistogram.o \
         QCamDisplay.o QCamFindShift.o QCamFindShift_barycentre.o \
         QTelescope.o QTelescopeAutostar.o QTelescopeAPM.o \
         QCamAutoGuidage.o QCamAutoGuidageSimple.o QCamAutoAlign.o \
         QCamMax.o QCamFindShift_hotSpot.o QVectorMap.o QCamComboBox.o \
         QCamTrans.o  FrameMirror.o FrameAlgo.o QCamOV511.o \
         QGridBox.o QDirectoryChooser.o QSetting.o QCamV4L2.o \
         QCamDisplayImplSDL.o QTelescopeFifo.o QFileChooser.o \
	 QKingClient.o QTelescopeMCU.o QTelescopeMTS.o QTelescopeFile.o \
	 FrameBayer.o FrameDark.o FrameFlat.o FrameBias.o QCamStack.o \
         FrameId.o QCamDC60.o QCamV4L2lx.o QCamV4L2fi.o QCamPhoto2.o \
         QCamFocus.o QCamSelection.o QCamQHY5.o

OBJ_cpp = QCamV4LbestDevice.o FitsImage.o QCamUtilities.o \
          QStreamTranslator.o Vector2D.o ShiftInfo.o QCamFrame.o \
	  QFrameDisplay.o rotate.o rasterOp.o SCmod.o bayer.o \
          QCamMovie.o QCamMovieAvi.o QCamMovieAviLossless.o QCamMovieSeq.o \
	  SettingsBackup.o SCmodParPortPPdev.o PPort.o yuv.o jmemsrc.o \
	  QHY5cam.o

OBJ_c =

include ../Makefile.common
