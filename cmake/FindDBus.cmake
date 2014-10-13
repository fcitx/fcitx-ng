#.rst:
# FindDBus
# -----------
#
# Read-Only variables:
#
# ::
#
#   DBUS_FOUND - system has the dbus-1 library
#   DBUS_INCLUDE_DIR - the dbus include directory
#   DBUS_LIBRARIES - The libraries needed to use OpenSSL
#   DBUS_VERSION - This is set to $major.$minor.$revision

#=============================================================================
# Copyright 2014-2014 Weng Xuetian <wengxt@gmail.com>
#

find_package(PkgConfig QUIET)
pkg_check_modules(PC_DBUS QUIET dbus-1)

find_path(DBUS_INCLUDE_DIR
  NAMES
    dbus/dbus.h
  HINTS
    ${PC_DBUS_INCLUDEDIR}
    ${PC_DBUS_INCLUDE_DIRS}
)

find_library(DBUS_LIBRARY
  NAMES
    dbus-1
  HINTS
    ${PC_DBUS_LIBDIR}
    ${PC_DBUS_LIBRARY_DIRS}
)

get_filename_component(_DBUS_LIBRARY_DIR ${DBUS_LIBRARY} PATH)
find_path(DBUS_ARCH_INCLUDE_DIR
    NAMES dbus/dbus-arch-deps.h
    HINTS ${PC_DBUS_INCLUDEDIR}
          ${PC_DBUS_INCLUDE_DIRS}
          ${_DBUS_LIBRARY_DIR}
          ${DBUS_INCLUDE_DIR}
    PATH_SUFFIXES include
)

mark_as_advanced(DBUS_LIBRARY)

set(DBUS_LIBRARIES ${DBUS_LIBRARY})
set(DBUS_INCLUDE_DIRS ${DBUS_INCLUDE_DIR} ${DBUS_ARCH_INCLUDE_DIR})

if (DBUS_INCLUDE_DIR)
  if (PC_DBUS_VERSION)
    set(DBUS_VERSION "${PC_DBUS_VERSION}")
  endif ()
endif ()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(DBus
  REQUIRED_VARS
    DBUS_LIBRARIES
    DBUS_INCLUDE_DIRS
  VERSION_VAR
    DBUS_VERSION
  FAIL_MESSAGE
    "Could NOT find dbus-1"
)

mark_as_advanced(DBUS_INCLUDE_DIRS DBUS_LIBRARIES)
