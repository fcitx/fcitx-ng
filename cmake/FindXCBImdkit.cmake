#.rst:
# FindXCBImdkit
# -----------
#
# Read-Only variables:
#
# ::
#
#   XCBIMDKIT_FOUND - system has the OpenSSL library
#   XCBIMDKIT_INCLUDE_DIR - the OpenSSL include directory
#   XCBIMDKIT_LIBRARIES - The libraries needed to use OpenSSL
#   XCBIMDKIT_VERSION - This is set to $major.$minor.$revision

#=============================================================================
# Copyright 2014-2014 Weng Xuetian <wengxt@gmail.com>
#

find_package(PkgConfig QUIET)
pkg_check_modules(PC_XCBIMDKIT QUIET xcb-imdkit)


find_path(XCBIMDKIT_INCLUDE_DIR
  NAMES
    xcb-imdkit/imdkit.h
  HINTS
    ${PC_XCBIMDKIT_INCLUDEDIR}
    ${PC_XCBIMDKIT_INCLUDE_DIRS}
)

find_library(XCBIMDKIT_LIBRARY
  NAMES
    xcb-imdkit
  HINTS
    ${PC_XCBIMDKIT_LIBDIR}
    ${PC_XCBIMDKIT_LIBRARY_DIRS}
)

mark_as_advanced(XCBIMDKIT_LIBRARY)

set(XCBIMDKIT_LIBRARIES ${XCBIMDKIT_LIBRARY})

if (XCBIMDKIT_INCLUDE_DIR)
  if (PC_XCBIMDKIT_VERSION)
    set(XCBIMDKIT_VERSION "${PC_XCBIMDKIT_VERSION}")
  endif ()
endif ()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(XCBImdkit
  REQUIRED_VARS
    XCBIMDKIT_LIBRARIES
    XCBIMDKIT_INCLUDE_DIR
  VERSION_VAR
    XCBIMDKIT_VERSION
  FAIL_MESSAGE
    "Could NOT find xcb-imdkit"
)

mark_as_advanced(XCBIMDKIT_INCLUDE_DIR XCBIMDKIT_LIBRARIES)
