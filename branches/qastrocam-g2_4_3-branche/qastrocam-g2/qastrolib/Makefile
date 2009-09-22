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
         QGridBox.o QDirectoryChooser.o \
         QCamDisplayImplSDL.o SCmodParPort.o SCmodParPortPPdev.o QTelescopeFifo.o \
	 QKingClient.o QTelescopeMCU.o QTelescopeMTS.o QTelescopeFile.o

OBJ_cpp = QCamV4LbestDevice.o port.o FitsImage.o QCamUtilities.o \
          QStreamTranslator.o PPort.o Vector2D.o ShiftInfo.o QCamFrame.o \
	  QFrameDisplay.o rotate.o rasterOp.o SCmod.o bayer.o \
          QCamMovie.o QCamMovieAvi.o QCamMovieSeq.o SettingsBackup.o

OBJ_c = ccvt_c2.o

include ../Makefile.common
