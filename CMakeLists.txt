#########################################
#      CMakeLists for qastrocam-g2      #
# (c) Blaise-Florentin Collin 2013-2014 #
#          under GPL v3 License         #
#########################################

cmake_minimum_required(VERSION 2.6.0)


####################
# BUILD DIRECTIVES #
####################

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
set(PATCH_VERSION "0")
set(BUILD_VERSION "198")
set(FULL_VERSION "${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D BINARY_VERSION=\\\"${FULL_VERSION}\\\" -D BUILD_VERSION=${BUILD_VERSION}")
message(STATUS "Building qastrocam-g2 ${FULL_VERSION}")

#
# distribution detection
# 
find_file(DEBIAN_FOUND debian_version debconf.conf PATHS /etc)
find_file(FEDORA_FOUND fedora-release PATHS /etc)
find_file(MAGEIA_FOUND mageia-release PATHS /etc)

set(DISTRIBUTION "Unknown")
if(DEBIAN_FOUND)
   set(DISTRIBUTION "Debian based")
endif(DEBIAN_FOUND)
if(FEDORA_FOUND)
   set(DISTRIBUTION "Fedora based")
endif(FEDORA_FOUND)
if(MAGEIA_FOUND)
   set(DISTRIBUTION "Mageia based")
endif(MAGEIA_FOUND)
message(STATUS "Host distribution : ${DISTRIBUTION}")

#
# kernel detection
#
execute_process(COMMAND  uname -r
        OUTPUT_VARIABLE KERNEL_OUTPUT
        OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX MATCH "[0-9]+" KERNEL_VERSION ${KERNEL_OUTPUT})
message(STATUS "Kernel detection : version ${KERNEL_VERSION}")
if(KERNEL_VERSION STREQUAL "2")
   add_definitions("-D KERNEL_2=1")
else(KERNEL_VERSION STREQUAL "2")
   add_definitions("-D KERNEL_2=0")
endif(KERNEL_VERSION STREQUAL "2")

#
# destdir for binary packages
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
# QT4 needed
#
find_package(Qt4 REQUIRED QtCore QtGui)
if(NOT QT_FOUND)
   message(FATAL_ERROR "Qt4 not found, generation stopped")
endif(NOT QT_FOUND)

#
# Libpthread recommended
#
find_package(Threads)
if(NOT Threads_FOUND)
   message(STATUS "Libpthreads not found, QHY5 and QHY6 camera's won't be supported...")
else(NOT Threads_FOUND)
   message(STATUS "Found libpthreads")
endif(NOT Threads_FOUND) 

#
# Libusb recommended
#
find_package(USB)
if(NOT USB_FOUND)
   message(STATUS "Libusb not found, QHY5 and QHY6 camera's won't be supported...")
endif(NOT USB_FOUND)

#
# libjpeg recommended
#
find_package(JPEG)
if(NOT JPEG_FOUND)
   message(STATUS "JPEG not found, jpeg cameras won't work...")
endif(NOT JPEG_FOUND)

#
# CFITSIO lib recommended
#
find_package(CFITSIO)
if(NOT CFITSIO_FOUND)
   message(STATUS "cfitsio libary not found, binary won't handle fits images...")
endif(NOT CFITSIO_FOUND)

#
# LibAV or AVIFile recommended
#
find_package(LibAV REQUIRED)
if(NOT LIBAV_FOUND)
   message(STATUS "")
   message(STATUS "On Fedora, libav is in the ffmpeg-compat-devel package on rpmfusion")
   message(STATUS "Try to add the '/usr/lib64/ffmpeg-compat/pkgconfig' in the PKG_CONFIG_PATH env var")
   message(STATUS "")
   message(STATUS "LibAv not found, trying libavifile...")
   #
   # looking for libavifile
   #
   #find_package(AVIFile)
   if(NOT AVIFILE_FOUND)
      message(STATUS "avifile library not found, binary won't handle avi files...")
   endif(NOT AVIFILE_FOUND)
else(NOT LIBAV_FOUND)
   set(AVIFILE_FOUND "")
endif(NOT LIBAV_FOUND)

#
# SDL recommended
#
find_package(SDL)
if(NOT SDL_FOUND)
   message(STATUS "SDL library not found, binary won't handle SDL...")
else(NOT SDL_FOUND)
   # X11 needed for Xsync
   find_package(X11)
   if(X11_FOUND)
      message(STATUS "Found SDL: ${SDL_LIBRARY}")
   else(X11_FOUND)
      set(SDL_FOUND "")
      message(STATUS "X11 not found, binary won't handle SDL...")
   endif(X11_FOUND)
endif(NOT SDL_FOUND)


#################
# Build sources #
#################

message(STATUS "Listing source files...")

# grab all source files
execute_process(COMMAND /bin/sh -c "ls src/*.cpp"
        OUTPUT_VARIABLE qastrocam-g2_SOURCES
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        OUTPUT_STRIP_TRAILING_WHITESPACE)

# transforms to a list
string(REPLACE "\n" ";" qastrocam-g2_SOURCES ${qastrocam-g2_SOURCES})


#
# MOC generation
#

message(STATUS "Listing MOC header files...")

#
# grab all the header files containing Q_OBJECT
#
execute_process(COMMAND /bin/sh -c "grep -l Q_OBJECT src/*.hpp"
	OUTPUT_VARIABLE qastrocam-g2_MOC_HEADERS
	WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
	OUTPUT_STRIP_TRAILING_WHITESPACE)

# transforms to a list
string(REPLACE "\n" ";" qastrocam-g2_MOC_HEADERS ${qastrocam-g2_MOC_HEADERS})

#
# exclude conditionnal stuff
#
if(KERNEL_VERSION STREQUAL "2")
   list(REMOVE_ITEM qastrocam-g2_MOC_HEADERS "src/QCamVestaK3.hpp")
else(KERNEL_VERSION STREQUAL "2")
   list(REMOVE_ITEM qastrocam-g2_MOC_HEADERS "src/QCamV4L.hpp" "src/QCamVestaK2.hpp")
endif(KERNEL_VERSION STREQUAL "2")

if(NOT (USB_FOUND AND Threads_FOUND))
   list(REMOVE_ITEM qastrocam-g2_MOC_HEADERS "src/QCamQHY5.hpp"
       "src/QCamQHY6.hpp"
       "src/QHY5cam.hpp"
       "src/QHY6cam.hpp"
       "src/QTelescopeQHY5.hpp"
       "src/QTelescopeQHY6.hpp"
   )
endif(NOT (USB_FOUND AND Threads_FOUND))

if(NOT SDL_FOUND)
   list(REMOVE_ITEM qastrocam-g2_MOC_HEADERS "src/QCamDisplayImplSDL.hpp")
endif(NOT SDL_FOUND)

#
# wrap to moc
#
set(qastrocam-g2_MOC_DEFINITIONS "")

# kernel 2
if(KERNEL_VERSION STREQUAL "2")
   set(qastrocam-g2_MOC_DEFINITIONS "${qastrocam-g2_MOC_DEFINITIONS}-DKERNEL_2=1;")
endif(KERNEL_VERSION STREQUAL "2")

# we use threads
if(Threads_FOUND)
   set(qastrocam-g2_MOC_DEFINITIONS "${qastrocam-g2_MOC_DEFINITIONS}-DHAVE_PTHREADS_H=1;")
endif(Threads_FOUND)

# we use usb
if(USB_FOUND)
   set(qastrocam-g2_MOC_DEFINITIONS "${qastrocam-g2_MOC_DEFINITIONS}-DHAVE_USB_H=1;")
endif(USB_FOUND)

# we use SDL
if(SDL_FOUND)
   set(qastrocam-g2_MOC_DEFINITIONS "${qastrocam-g2_MOC_DEFINITIONS}-DHAVE_SDL_H=1;")
endif(SDL_FOUND)

# wrap
QT4_WRAP_CPP(qastrocam-g2_MOC_CPP ${qastrocam-g2_MOC_HEADERS} OPTIONS ${qastrocam-g2_MOC_DEFINITIONS})


############################
# Handling libraries datas #
############################

#
# we use Qt4
#
include(${QT_USE_FILE})
set(LIBRARIES_LIST ${QT_LIBRARIES})

#
# we use libjpeg
#

if(JPEG_FOUND)
   include_directories(${JPEG_INCLUDE_DIR})
   set(USB_DEFINITIONS "${JPEG_DEFINITIONS} -D HAVE_JPEG_H=1")
   add_definitions(${JPEG_DEFINITIONS})
   set(LIBRARIES_LIST ${LIBRARIES_LIST} ${JPEG_LIBRARIES})
endif(JPEG_FOUND)

#
# we use libpthreads
#
if(Threads_FOUND)
   set(THREADS_DEFINITIONS "-D HAVE_PTHREADS_H=1")
   add_definitions(${THREADS_DEFINITIONS})
   set(LIBRARIES_LIST ${LIBRARIES_LIST} "-lpthread")
endif(Threads_FOUND)

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
# we use LibAV
#
if(LIBAV_FOUND)
   include_directories(${LIBAV_INCLUDE_DIRS})
   set(LIBAV_DEFINITIONS "${LIBAV_DEFINITIONS} -D HAVE_LIBAV_H=1 -D __STDC_CONSTANT_MACROS")
   add_definitions(${LIBAV_DEFINITIONS})
   set(LIBRARIES_LIST ${LIBRARIES_LIST} ${LIBAV_LIBRARIES})
endif(LIBAV_FOUND)

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
   # SDL stuff
   include_directories(${SDL_INCLUDE_DIR})
   set(SDL_DEFINITIONS "${SDL_DEFINITIONS} -D HAVE_SDL_H=1")
   add_definitions(${SDL_DEFINITIONS})
   set(LIBRARIES_LIST ${LIBRARIES_LIST} ${SDL_LIBRARY})
   # X11 stuff
   include_directories(${X11_INCLUDE_DIR})
   set(LIBRARIES_LIST ${LIBRARIES_LIST} ${X11_LIBRARIES})
endif(SDL_FOUND)


#####################
# Target definition #
#####################

#
# target
#
add_executable(qastrocam-g2 ${qastrocam-g2_SOURCES} ${qastrocam-g2_MOC_CPP})

#
# target libs
#
target_link_libraries(qastrocam-g2 ${LIBRARIES_LIST})

#
# add a target to generate API documentation with Doxygen
#
find_package(Doxygen)
if(DOXYGEN_FOUND)
   configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
   add_custom_target(doc ALL ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating API documentation with Doxygen" VERBATIM
      )
endif(DOXYGEN_FOUND)

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
install(FILES icons/qastrocam-g2-icon-nb.png DESTINATION /usr/share/qastrocam-g2/icons)
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
install(FILES doc/Contributors DESTINATION /usr/share/qastrocam-g2)
install(FILES doc/ChangeLog DESTINATION /usr/share/qastrocam-g2)
install(FILES doc/License.txt DESTINATION /usr/share/qastrocam-g2)
install(FILES doc/README DESTINATION /usr/share/qastrocam-g2)
install(FILES doc/README.philips DESTINATION /usr/share/qastrocam-g2)
install(FILES doc/README.qhy DESTINATION /usr/share/qastrocam-g2)
install(FILES doc/dev-notes DESTINATION /usr/share/qastrocam-g2)

# install desktop entry
install(FILES qastrocam-g2.desktop DESTINATION /usr/share/applications)

# install avi lossless compression dll
if(AVIFILE_FOUND)
   install(FILES lib/huffyuv.dll DESTINATION /usr/lib/win32)
   install(FILES lib/huffyuv_readme.txt DESTINATION /usr/share/qastrocam-g2)
endif(AVIFILE_FOUND)

# install QHY driver
install(FILES qhy_driver/qhyccd.rules DESTINATION /etc/udev/rules.d)
install(FILES qhy_driver/QHY5.HEX DESTINATION /usr/share/qastrocam-g2/qhy_driver)
install(FILES qhy_driver/QHY5LOADER.HEX DESTINATION /usr/share/qastrocam-g2/qhy_driver)
install(FILES qhy_driver/QHY6.HEX DESTINATION /usr/share/qastrocam-g2/qhy_driver)
install(FILES qhy_driver/LICENSE DESTINATION /usr/share/qastrocam-g2/qhy_driver)

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
# Source package #
##################

#
# source package
#
set(CPACK_SOURCE_GENERATOR TGZ)
set(CPACK_SOURCE_IGNORE_FILES ".svn")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "qastrocam-g2_${FULL_VERSION}" CACHE INTERNAL "tarball basename")


#####################
# Binaries packages #
#####################

# global directives for debian and rpm packages
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Video capture software for astronomy")
set(CPACK_PACKAGE_DESCRIPTION "qastrocam-g2 is a tool for astronomy imaging capture/processing. ")
set(CPACK_PACKAGE_DESCRIPTION "${CPACK_PACKAGE_DESCRIPTION}It handles any V4L device and philips cameras special features.")
set(CPACK_PACKAGE_DESCRIPTION "${CPACK_PACKAGE_DESCRIPTION} Many special modules like guiding and stacking are provided.")
set(CPACK_PACKAGE_DESCRIPTION "${CPACK_PACKAGE_DESCRIPTION} This version is a fork from the F. Sicard's qastrocam.")
set(CPACK_PACKAGE_VENDOR "Thx8411")
set(CPACK_PACKAGE_CONTACT "Blaise-Florentin Collin <thx8411@users.sourceforge.net>")
set(CPACK_STRIP_FILES qastrocam-g2)


##################
# Debian package #
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
   set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libqtcore4, libqtgui4")

   # recommended dependencies
   if(JPEG_FOUND)
      set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libjpeg62")
   endif(JPEG_FOUND)

   if(USB_FOUND AND Threads_FOUND)
      set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libusb-0.1-4")
      # we need fxload for the qhy driver
      set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, fxload")
   endif(USB_FOUND AND Threads_FOUND)

   if(CFITSIO_FOUND)
      set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libcfitsio3")
   endif(CFITSIO_FOUND)

   if(LIBAV_FOUND)
      set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libavformat52 | libavformat-extra-52") 
      set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}| libavformat53 | libavformat-extra-53") 
      set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}| libavformat54 | libavformat-extra-54")
      set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libavcodec52 | libavcodec-extra-52")
      set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}| libavcodec53 | libavcodec-extra-53") 
      set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}| libavcodec54 | libavcodec-extra-54")
      set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libavutil49 | libavutil-extra-49")
      set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}| libavutil50 | libavutil-extra-50")
      set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}| libavutil51 | libavutil-extra-51")
      set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}| libavutil52 | libavutil-extra-52")
   endif(LIBAV_FOUND)

   if(AVIFILE_FOUND)
      set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libavifile-0.7c2")
      # recommended for lossless video compression
      set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "avifile-win32-plugin")
   endif(AVIFILE_FOUND)

   if(SDL_FOUND)
      set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libsdl1.2debian")
   else(SDL_FOUND)
      # recommended for AutoGuiding sound alerts
      if(AVIFILE_FOUND)
         set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "${CPACK_DEBIAN_PACKAGE_RECOMMENDS}, nas")
      else(AVIFILE_FOUND)
         set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "nas")
      endif(AVIFILE_FOUND)
   endif(SDL_FOUND)

   #
   # package infos
   #
   set(CPACK_DEBIAN_PACKAGE_SECTION graphics)
   set(CPACK_DEBIAN_PACKAGE_VERSION "${DEB_EPOCH}:${FULL_VERSION}-1")
   set(CPACK_PACKAGE_FILE_NAME ${CPACK_DEBIAN_PACKAGE_NAME}_${FULL_VERSION}_${ARCHITECTURE}_k${KERNEL_VERSION})
   set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "${CPACK_PACKAGE_DESCRIPTION_SUMMARY}\n ${CPACK_PACKAGE_DESCRIPTION}")

endif(DEBIAN_FOUND)


###############
# Rpm package #
###############


#
# TODO :
# check for avifile-win32-plugin and nas on mageia and fedora
#

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
   set(CPACK_RPM_PACKAGE_LICENSE "GPL v2")
   set(CPACK_RPM_PACKAGE_GROUP "Productivity/Scientific/Astronomy")
   set(CPACK_RPM_PACKAGE_DESCRIPTION ${CPACK_PACKAGE_DESCRIPTION})
   set(CPACK_RPM_PACKAGE_NAME "qastrocam-g2")
   set(CPACK_PACKAGE_FILE_NAME ${CPACK_RPM_PACKAGE_NAME}_${FULL_VERSION}.${ARCHITECTURE}_k${KERNEL_VERSION})
   set(CPACK_RPM_PACKAGE_RELOCATABLE FALSE)

   #
   # dependencies
   #

   if(MAGEIA_FOUND)

      if(ARCHITECTURE STREQUAL "x86_64")

         # base dependencies
         set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}lib64qtcore4, lib64qtgui4")

         # recommended dependencies
         if(JPEG_FOUND)
            set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, lib64jpeg62")
         endif(JPEG_FOUND)

         if(USB_FOUND AND Threads_FOUND)
            set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, lib64usb-compat0.1_4, fxload")
         endif(USB_FOUND AND Threads_FOUND)

         if(CFITSIO_FOUND)
           set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, lib64cfitsio1")
         endif(CFITSIO_FOUND)

         if(LIBAV_FOUND)
            set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, lib64avformat55, lib64avcodec55, lib64avutil52")
         endif(LIBAV_FOUND)

         if(AVIFILE_FOUND)
            set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, lib64avifile-0.7c2")
         endif(AVIFILE_FOUND)

         if(SDL_FOUND)
            set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, lib64SDL1.2_0")
         else(SDL_FOUND)
       	    set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, nas")
         endif(SDL_FOUND)

      else(ARCHITECTURE STREQUAL "x86_64")

         # base dependencies
         set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}libqtcore4, libqtgui4")

         # recommended dependencies
         if(JPEG_FOUND)
            set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libjpeg62")
         endif(JPEG_FOUND)

         if(USB_FOUND AND Threads_FOUND)
            set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libusb-compat0.1_4, fxload")
         endif(USB_FOUND AND Threads_FOUND)

         if(CFITSIO_FOUND)
            set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libcfitsio1")
         endif(CFITSIO_FOUND)

         if(LIBAV_FOUND)
            set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libavformat55, libavcodec55, libavutil52")
         endif(LIBAV_FOUND)

         if(AVIFILE_FOUND)
            set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libavifile-0.7c2")
         endif(AVIFILE_FOUND)

         if(SDL_FOUND)
            set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libSDL1.2_0")
         else(SDL_FOUND)
       	    set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, nas")
         endif(SDL_FOUND)

      endif(ARCHITECTURE STREQUAL "x86_64")

   else(MAGEIA_FOUND)

      # base dependencies
      set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}qt, qt-x11")

      # recommended dependencies
      if(JPEG_FOUND)
         set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libjpeg-turbo")
      endif(JPEG_FOUND)

      if(USB_FOUND AND Threads_FOUND)
         set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libusb, fxload")
      endif(USB_FOUND AND Threads_FOUND)

      if(CFITSIO_FOUND)
         set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, cfitsio")
      endif(CFITSIO_FOUND)

      if(LIBAV_FOUND)
         set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, ffmpeg-compat")
      endif(LIBAV_FOUND)

      if(AVIFILE_FOUND)
         set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libavifile-0.7c2")
      endif(AVIFILE_FOUND)

      if(SDL_FOUND)
         set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, SDL")
      else(SDL_FOUND)
         set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, nas")
      endif(SDL_FOUND)

      # TODO : defined for cmake 2.8.10 and above only ?
      set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST /etc /usr /usr/bin /usr/share /usr/share/applications /usr/share/icons /etc/udev /etc/udev/rules.d)

   endif(MAGEIA_FOUND)

endif(FEDORA_FOUND OR MAGEIA_FOUND)

#
# package module
#
include(CPack)


#########
# TESTS #
#########

include(CTest)
