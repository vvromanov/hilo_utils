#.rst:
# FindLibEv
# -----------
#
# Try to find the LibEv library
#
# Once done this will define
#
# ::
#
#   LIBEV_FOUND - System has LibEv
#   LIBEV_INCLUDE_DIR - The LibEv include directory
#   LIBEV_LIBRARIES - The libraries needed to use LibEv

#=============================================================================
# Copyright 2006-2009 Kitware, Inc.
# Copyright 2006 Alexander Neundorf <neundorf@kde.org>
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

find_path(LIBEV_INCLUDE_DIR NAMES ev.h
   )

if(CYGWIN)
    find_library(LIBEV_LIBRARIES NAMES libev.dll.a)
else(CYGWIN)
    find_library(LIBEV_LIBRARIES NAMES libev.so)
endif(CYGWIN)
# handle the QUIETLY and REQUIRED arguments and set LIBEV_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibEv
                                  REQUIRED_VARS LIBEV_LIBRARIES LIBEV_INCLUDE_DIR
                                  VERSION_VAR LIBEV_VERSION_STRING)

mark_as_advanced(LIBEV_INCLUDE_DIR LIBEV_LIBRARIES)
