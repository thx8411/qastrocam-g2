# - Locate AVIFILE library and include paths
# AVIFILE (http://avifile.sourceforge.net/)is a set of libraries for
# i386 machines
# to use various AVI codecs. Support is limited beyond Linux. Windows
# provides native AVI support, and so doesn't need this library.
# This module defines
#  AVIFILE_INCLUDE_DIR, where to find avifile.h , etc.
#  AVIFILE_LIBRARIES, the libraries to link against
#  AVIFILE_DEFINITIONS, definitions to use when compiling
#  AVIFILE_FOUND, If false, don't try to use AVIFILE

#=============================================================================
# Copyright 2002-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

if(UNIX)
  find_path(AVIFILE_INCLUDE_DIR avifile.h
    /usr/include/avifile
    /usr/local/avifile/include
    /usr/local/include/avifile
  )

  find_library(AVIFILE_AVIPLAY_LIBRARY aviplay
    /usr/local/avifile/lib
  )
endif(UNIX)

set(AVIFILE_FOUND FALSE)

if(AVIFILE_INCLUDE_DIR AND AVIFILE_AVIPLAY_LIBRARY)
   set(AVIFILE_FOUND TRUE)
   set(AVIFILE_LIBRARIES ${AVIFILE_AVIPLAY_LIBRARY})
   set(AVIFILE_DEFINITIONS "")
   message(STATUS "Found AVIFile: ${AVIFILE_LIBRARIES}")
endif(AVIFILE_INCLUDE_DIR AND AVIFILE_AVIPLAY_LIBRARY)

mark_as_advanced(AVIFILE_INCLUDE_DIR AVIFILE_AVIPLAY_LIBRARY)
