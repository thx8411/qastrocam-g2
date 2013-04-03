####################################
#   CMakeLists for qastrocam-g2    #
# (c) Blaise-Florentin Collin 2013 #
#       under GPL v3 License       #
####################################  

cmake_minimum_required(VERSION 2.6.8)


####################
# BUILD DIRECTIVES #
####################

#
# distribution detection
#
find_file(DEBIAN_FOUND debian_version debconf.conf PATHS /etc)
find_file(FEDORA_FOUND fedora-release PATHS /etc)
find_file(MAGEIA_FOUND mageia-release PATHS /etc)

#
# kernel detection
#
execute_process(COMMAND  uname -r
      OUTPUT_VARIABLE KERNEL_OUTPUT
      OUTPUT_STRIP_TRAILING_WHITESPACE
   )
string(REGEX MATCH "[0-9]+" KERNEL_VERSION ${KERNEL_OUTPUT})
message(STATUS "Kernel detection : version ${KERNEL_VERSION}")
if(KERNEL_VERSION STREQUAL "2")
   add_definitions("-D KERNEL_2=1")
else(KERNEL_VERSION STREQUAL "2")
   add_definitions("-D KERNEL_2=0")
endif(KERNEL_VERSION STREQUAL "2")

#
# project name
#
project(qastrocam-g2)

#
# version
#
set(DEB_EPOCH 0)
set(MAJOR_VERSION "5")
set(MINOR_VERSION "0")
set(PATCH_VERSION "beta20130403")
set(BUILD_VERSION "164")
set(FULL_VERSION "${MAJOR_VERSION}.${MINOR_VERSION}${PATCH_VERSION}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D BINARY_VERSION=\\\"${FULL_VERSION}\\\" -D BUILD_VERSION=${BUILD_VERSION}")

#
# destir for binary packages
#
set($ENV{DESTDIR} "${CMAKE_BINARY_DIR}/fakeinstall")

#
# add cmake module path
#
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")


##########################
# Libraries dependencies #
##########################

#
# QT3 needed
#
find_package(Qt3 REQUIRED)
if(NOT QT3_FOUND)
   message(FATAL_ERROR "Qt3 not found, generation stopped")
endif(NOT QT3_FOUND)

#
# libjpeg needed
#
find_package(JPEG REQUIRED)
if(NOT JPEG_FOUND)
   message(FATAL_ERROR "JPEG not found, generation stopped")
endif(NOT JPEG_FOUND)

#
# Libusb needed
#
find_package(USB REQUIRED)
if(NOT USB_FOUND)
   message(FATAL_ERROR "Libusb not found, generation stopped")
endif(NOT USB_FOUND)

#
# CFITSIO lib recommended
#
find_package(CFITSIO)
if(NOT CFITSIO_FOUND)
   message(STATUS "cfitsio libary not found, binary won'y handle fits images...")
endif(NOT CFITSIO_FOUND)

#
# AVIFile lib recommended
#
find_package(AVIFile)
if(NOT AVIFILE_FOUND)
   message(STATUS "avifile library not found, binary won't handle avi files...")
endif(NOT AVIFILE_FOUND)

#
# SDL recommended
#
find_package(SDL)
if(NOT SDL_FOUND)
   message(FATAL_ERROR "SDL library not found, binary won't handle SDL...")
else(NOT SDL_FOUND)
   message(STATUS "Found SDL: ${SDL_LIBRARY}")
endif(NOT SDL_FOUND)


#################
# Build sources #
#################

# source files
set(qastrocam-g2_SOURCES
	src/bayer.cpp
	src/QCamDisplay.cpp
	src/QCamStack.cpp
	src/QTelescopeAPM.cpp
	src/CamHistogram.cpp
	src/QCamDisplayImplSDL.cpp
	src/QCamTrans.cpp
	src/QTelescopeAutostar.cpp
	src/FitsImage.cpp
	src/QCamFindShift_barycentre.cpp
	src/QCamUtilities.cpp
	src/QTelescope.cpp
	src/FrameAlgo.cpp
	src/QCamFindShift.cpp
	src/QCamV4L2.cpp
	src/QTelescopeFifo.cpp
	src/FrameBayer.cpp
	src/QCamFindShift_hotSpot.cpp
	src/QCamV4L2fi.cpp
	src/QTelescopeFile.cpp
	src/FrameBias.cpp
	src/QCamFocus.cpp
	src/QCamV4L2lx.cpp
	src/QTelescopeLX200.cpp
	src/FrameDark.cpp
	src/QCamFrame.cpp
	src/QCamV4LbestDevice.cpp
	src/QTelescopeMCU.cpp
	src/FrameFlat.cpp
	src/QCamMax.cpp
	src/QCamV4L.cpp
	src/QTelescopeMTS.cpp
	src/FrameId.cpp
	src/QCamMovieAvi.cpp
	src/QCamVesta.cpp
	src/QTelescopeNexstar.cpp
	src/FrameMirror.cpp
	src/QCamMovieAviLossless.cpp
	src/QTelescopeQHY5.cpp
	src/jmemsrc.cpp
	src/QCamMovie.cpp
	src/QTelescopeQHY6.cpp
	src/main.cpp
	src/QCamMovieSeq.cpp
	src/QDirectoryChooser.cpp
	src/QTelescopeSimulator.cpp
	src/PPort.cpp
	src/QCamMovieSer.cpp
	src/QFileChooser.cpp
	src/QVectorMap.cpp
	src/QCamAdd.cpp
	src/QCamOV511.cpp
	src/QFrameDisplay.cpp
	src/rasterOp.cpp
	src/QCamAutoAlign.cpp
	src/QCamPhoto2.cpp
	src/QGridBox.cpp
	src/rotate.cpp
	src/QCamAutoGuidage.cpp
	src/QCamQHY5.cpp
	src/QHistogram.cpp
	src/SCmod.cpp
	src/QCamAutoGuidageSimple.cpp
	src/QCamQHY6.cpp
	src/QHY5cam.cpp
	src/SCmodParPortPPdev.cpp
	src/QCamClient.cpp
	src/QCamRadioBox.cpp
	src/QHY6cam.cpp
	src/SettingsBackup.cpp
	src/QCamComboBox.cpp
	src/QCamSelection.cpp
	src/QKingClient.cpp
	src/ShiftInfo.cpp
	src/QCam.cpp
	src/QCamSimulator.cpp
	src/QSetting.cpp
	src/Vector2D.cpp
	src/QCamDC60.cpp
	src/QCamSlider.cpp
	src/QStreamTranslator.cpp
	src/yuv.cpp
)

# moc files generation
set(qastrocam-g2_MOC
	CamHistogram
	FrameAlgo
	FrameBayer
	FrameBias
	FrameDark
	FrameFlat
	FrameId
	FrameMirror
	QCamAdd
	QCamAutoAlign
	QCamAutoGuidage
	QCamAutoGuidageSimple
	QCamClient
	QCamComboBox
	QCamDC60
	QCamDisplay
	QCamDisplayImplSDL
	QCamFindShift_barycentre
	QCamFindShift_hotSpot
	QCamFindShift
	QCamFocus
	QCam
	QCamMax
	QCamOV511
	QCamPhoto2
	QCamQHY5
	QCamQHY6
	QCamRadioBox
	QCamSelection
	QCamSimulator
	QCamSlider
	QCamStack
	QCamTrans
	QCamV4L2fi
	QCamV4L2
	QCamV4L2lx
	QCamV4L
	QCamVestaK2
	QCamVestaK3
	QDirectoryChooser
	QFileChooser
	QGridBox
	QHistogram
	QKingClient
	QSetting
	QTelescopeAPM
	QTelescopeAutostar
	QTelescopeFifo
	QTelescopeFile
	QTelescope
	QTelescopeLX200
	QTelescopeMCU
	QTelescopeMTS
	QTelescopeNexstar
	QTelescopeQHY5
	QTelescopeQHY6
	QTelescopeSimulator
	QVectorMap
)
#
# MOC generation
#
message(STATUS "MOC files generation...")
foreach(F ${qastrocam-g2_MOC})
   # ---------------------------------
   # TEMPORARY
   # must be replaced by a target rule
   # ---------------------------------
   execute_process(COMMAND ${QT_MOC_EXECUTABLE} "-o" "${CMAKE_BINARY_DIR}/${F}.moc" "${CMAKE_SOURCE_DIR}/src/${F}.hpp")
endforeach(F)
include_directories(${CMAKE_BINARY_DIR})


############################
# handling libraries datas #
############################

#
# we use Qt3
#
include_directories(${QT_INCLUDE_DIR} ${QT_INCLUDE_PATH})
add_definitions(${QT_DEFINITIONS})
set(LIBRARIES_LIST ${QT_LIBRARIES})

#
# we use libjpeg
#
#include_directories(${JPEG_INCLUDE_DIR})
#set(LIBRARIES_LIST ${LIBRARIES_LIST} ${JPEG_LIBRARIES})

#
# we use Libusb
#
if(USB_FOUND)
   include_directories(${USB_INCLUDE_DIRS})
   set(USB_DEFINITIONS "${USB_DEFINITIONS} -D HAVE_USB_H=1")
   add_definitions(${USB_DEFINITIONS})
   set(LIBRARIES_LIST ${LIBRARIES_LIST} ${USB_LIBRARIES})
endif(USB_FOUND)

#
# we use CFITSIO
#
if(CFITSIO_FOUND)
   include_directories(${CFITSIO_INCLUDE_DIR})
   set(CFITSIO_DEFINITIONS "${CFITSIO_DEFINITIONS} -D HAVE_CFITSIO_H=1")
   add_definitions(${CFITSIO_DEFINITIONS})
   set(LIBRARIES_LIST ${LIBRARIES_LIST} ${CFITSIO_LIBRARIES})
endif(CFITSIO_FOUND)

#
# we use AVIFile
#
if(AVIFILE_FOUND)
   include_directories(${AVIFILE_INCLUDE_DIR})
   set(AVIFILE_DEFINITIONS "${AVIFILE_DEFINITIONS} -D HAVE_AVIFILE_H=1")
   add_definitions(${AVIFILE_DEFINITIONS})
   set(LIBRARIES_LIST ${LIBRARIES_LIST} ${AVIFILE_LIBRARIES})
endif(AVIFILE_FOUND)

#
# we use SDL
#
if(SDL_FOUND)
   include_directories(${SDL_INCLUDE_DIR})
   set(SDL_DEFINITIONS "${SDL_DEFINITIONS} -D HAVE_SDL_H=1")
   add_definitions(${SDL_DEFINITIONS})
   set(LIBRARIES_LIST ${LIBRARIES_LIST} ${SDL_LIBRARY})
endif(SDL_FOUND)


#####################
# Target definition #
#####################

#
# target
#
add_executable(qastrocam-g2 ${qastrocam-g2_SOURCES})

#
# target libs
#
target_link_libraries(qastrocam-g2 ${LIBRARIES_LIST})

#
# installation
#

# install binary
install(TARGETS qastrocam-g2 RUNTIME DESTINATION /usr/bin)

# install icons
install(FILES icons/qastrocam-g2-icon.png DESTINATION /usr/share/icons)
install(FILES icons/choose_directory.png DESTINATION /usr/share/qastrocam-g2/icons)
install(FILES icons/directions.png DESTINATION /usr/share/qastrocam-g2/icons)
install(FILES icons/displayFrames.png DESTINATION /usr/share/qastrocam-g2/icons)
install(FILES icons/displayHistogram.png DESTINATION /usr/share/qastrocam-g2/icons)
install(FILES icons/down.png DESTINATION /usr/share/qastrocam-g2/icons)
install(FILES icons/east.png DESTINATION /usr/share/qastrocam-g2/icons)
install(FILES icons/exit.png DESTINATION /usr/share/qastrocam-g2/icons)
install(FILES icons/left.png DESTINATION /usr/share/qastrocam-g2/icons)
install(FILES icons/movie_pause.png DESTINATION /usr/share/qastrocam-g2/icons)
install(FILES icons/movie.png DESTINATION /usr/share/qastrocam-g2/icons)
install(FILES icons/north.png DESTINATION /usr/share/qastrocam-g2/icons)
install(FILES icons/qastrocam-g2-icon.png DESTINATION /usr/share/qastrocam-g2/icons)
install(FILES icons/qastrocam-icon-nb.png DESTINATION /usr/share/qastrocam-g2/icons)
install(FILES icons/right.png DESTINATION /usr/share/qastrocam-g2/icons)
install(FILES icons/snapshot.png DESTINATION /usr/share/qastrocam-g2/icons)
install(FILES icons/south.png DESTINATION /usr/share/qastrocam-g2/icons)
install(FILES icons/target_icon.png DESTINATION /usr/share/qastrocam-g2/icons)
install(FILES icons/up.png DESTINATION /usr/share/qastrocam-g2/icons)
install(FILES icons/west.png DESTINATION /usr/share/qastrocam-g2/icons)

#install sounds
install(FILES sounds/bell.wav DESTINATION /usr/share/qastrocam-g2/sounds)

# install doc files
install(FILES doc/qastrocam-g2-help.html DESTINATION /usr/share/qastrocam-g2)
install(FILES doc/BUGS DESTINATION /usr/share/qastrocam-g2)
install(FILES doc/TODO DESTINATION /usr/share/qastrocam-g2)
install(FILES doc/Contributors DESTINATION /usr/share/qastrocamg-2)
install(FILES doc/ChangeLog DESTINATION /usr/share/qastrocam-g2)
install(FILES doc/License.txt DESTINATION /usr/share/qastrocam-g2)
install(FILES doc/README DESTINATION /usr/share/qastrocam-g2)
install(FILES doc/README.philips DESTINATION /usr/share/qastrocam-g2)
install(FILES doc/dev-notes DESTINATION /usr/share/qastrocam-g2)

# install desktop entry
install(FILES qastrocam-g2.desktop DESTINATION /usr/share/applications)

# install avi lossless compression dll
install(FILES lib/huffyuv.dll DESTINATION /usr/lib/win32)
install(FILES lib/huffyuv_readme.txt DESTINATION /usr/share/qastrocam-g2)

# install file node
install(CODE "execute_process(COMMAND \"rm -f /tmp/qastrocam-g2_shift.fifo\")")
install(CODE "execute_process(COMMAND \"mkfifo /tmp/qastrocam-g2_shift.fifo\")")


######################
# PACKAGE GENERATION #
######################

#
# default arch
#
set(ARCHITECTURE "unkown")

#
# global directives
#
set(CPACK_PACKAGE_VERSION_MAJOR ${MAJOR_VERSION})
set(CPACK_PACKAGE_VERSION_MINOR ${MINOR_VERSION})
set(CPACK_PACKAGE_VERSION_PATCH ${PATCH_VERSION})


##################
# source package #
##################

#
# source package
#
set(CPACK_SOURCE_GENERATOR TGZ)
set(CPACK_SOURCE_IGNORE_FILES ".svn")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "qastrocam-g2_${FULL_VERSION}_src" CACHE INTERNAL "tarball basename")


#####################
# binaries packages #
#####################

# global directives for debian and rpm packages
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Video capture software for astronomy")
set(CPACK_PACKAGE_DESCRIPTION "qastrocam-g2 is a tool for astronomy imaging capture/processing. ")
set(CPACK_PACKAGE_DESCRIPTION "${CPACK_PACKAGE_DESCRIPTION}It handles any V4L device and philips cameras special features.")
set(CPACK_PACKAGE_DESCRIPTION "${CPACK_PACKAGE_DESCRIPTION} Many special modules like guiding and stacking are provided.")
set(CPACK_PACKAGE_DESCRIPTION "${CPACK_PACKAGE_DESCRIPTION} This version is a fork from the F. Sicard's qastrocam.")
set(CPACK_PACKAGE_VENDOR "Thx8411")
set(CPACK_PACKAGE_CONTACT "thx8411@users.sourceforge.net")
set(CPACK_STRIP_FILES qastrocam-g2)


##################
# debian package #
##################

if(DEBIAN_FOUND)

   #
   # arch detection
   #
   find_program(DPKG_PROGRAM dpkg DOC "dpkg program of Debian-based systems")
   if(DPKG_PROGRAM)
     execute_process(
        COMMAND ${DPKG_PROGRAM} --print-architecture
        OUTPUT_VARIABLE ARCHITECTURE
        OUTPUT_STRIP_TRAILING_WHITESPACE
     )
   endif(DPKG_PROGRAM)

   #
   # settings
   #
   set(CPACK_GENERATOR DEB)
   set(CPACK_SET_DESTDIR "ON")
   set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE ${ARCHITECTURE})
   set(CPACK_DEBIAN_PACKAGE_NAME "qastrocam-g2")

   #
   # dependencies
   #

   # base dependencies
   set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}libc6")
   set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libgcc1")
   set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libstdc++6")
   set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libx11-6")

   # required dependencies
   if(QT3_FOUND)
      set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libqt3-mt")
   endif(QT3_FOUND)

   if(JPEG_FOUND)
      set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libjpeg62")
   endif(JPEG_FOUND)

   if(USB_FOUND)
      set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libusb-0.1-4")
   endif(USB_FOUND)

   # recommended dependencies
   if(CFITSIO_FOUND)
      set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libcfitsio3")
   endif(CFITSIO_FOUND)

   if(AVIFILE_FOUND)
      set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libavifile-0.7c2")
   endif(AVIFILE_FOUND)

   if(SDL_FOUND)
      set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libsdl1.2debian")
   endif(SDL_FOUND)

   # other recommended dependencies (not needed for build)
   set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "avifile-win32-plugin, nas")

   #
   # package infos
   #
   set(CPACK_DEBIAN_PACKAGE_SECTION graphics)
   set(CPACK_DEBIAN_PACKAGE_VERSION "${DEB_EPOCH}:${FULL_VERSION}-1")
   set(CPACK_PACKAGE_FILE_NAME ${CPACK_DEBIAN_PACKAGE_NAME}_${FULL_VERSION}_${ARCHITECTURE}_k${KERNEL_VERSION})
   set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "${CPACK_PACKAGE_DESCRIPTION_SUMMARY}\n ${CPACK_PACKAGE_DESCRIPTION}")

endif(DEBIAN_FOUND)


###############
# rpm package #
###############

if(FEDORA_FOUND OR MAGEIA_FOUND)

   #
   # arch detection
   #
   execute_process(COMMAND  uname -m
      OUTPUT_VARIABLE ARCHITECTURE
      OUTPUT_STRIP_TRAILING_WHITESPACE
   )

   #
   # settings
   #
   set(CPACK_GENERATOR RPM)
   set(CPACK_SET_DESTDIR "OFF")
   set(CPACK_RPM_PACKAGE_ARCHITECTURE ${ARCHITECTURE})
   set(CPACK_RPM_PACKAGE_LICENSE "GPL v3")
   set(CPACK_RPM_PACKAGE_GROUP "Productivity/Scientific/Astronomy")
   set(CPACK_RPM_PACKAGE_DESCRIPTION ${CPACK_PACKAGE_DESCRIPTION})
   set(CPACK_RPM_PACKAGE_NAME "qastrocam-g2")
   set(CPACK_PACKAGE_FILE_NAME ${CPACK_RPM_PACKAGE_NAME}_${FULL_VERSION}.${ARCHITECTURE}_k${KERNEL_VERSION})
   set(CPACK_RPM_PACKAGE_RELOCATABLE FALSE)

   #
   # dependencies
   #

   #-----------------------------------------#
   #- TODO : package names have to be fixed -#
   #-----------------------------------------#
   if(ARCHITECURE STREQUAL "x86_64")
      # base dependencies
      set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}libc6")
      set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libgcc1")
      set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libstdc++6")
      set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libx11-6")

      # required dependencies
      if(QT3_FOUND)
         set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libqt3-mt")
      endif(QT3_FOUND)

      if(JPEG_FOUND)
         set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libjpeg62")
      endif(JPEG_FOUND)

      if(USB_FOUND)
         set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libusb-0.1-4")
      endif(USB_FOUND)

      # recommended dependencies
      if(CFITSIO_FOUND)
         set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libcfitsio3")
      endif(CFITSIO_FOUND)

      if(AVIFILE_FOUND)
         set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libavifile-0.7c2")
      endif(AVIFILE_FOUND)

      if(SDL_FOUND)
         set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libsdl1.2debian")
      endif(SDL_FOUND)

   else(ARCHITECURE STREQUAL "x86_64")

      # base dependencies
      set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}libc6")
      set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libgcc1")
      set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libstdc++6")
      set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libx11-6")

      # required dependencies
      if(QT3_FOUND)
         set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libqt3-mt")
      endif(QT3_FOUND)

      if(JPEG_FOUND)
         set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libjpeg62")
      endif(JPEG_FOUND)

      if(USB_FOUND)
         set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libusb-0.1-4")
      endif(USB_FOUND)

      # recommended dependencies
      if(CFITSIO_FOUND)
         set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libcfitsio3")
      endif(CFITSIO_FOUND)

      if(AVIFILE_FOUND)
         set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libavifile-0.7c2")
      endif(AVIFILE_FOUND)

      if(SDL_FOUND)
         set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libsdl1.2debian")
      endif(SDL_FOUND)

   endif(ARCHITECURE STREQUAL "x86_64")

endif(FEDORA_FOUND OR MAGEIA_FOUND)

#
# package module
#
include(CPack)
