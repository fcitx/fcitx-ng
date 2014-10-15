#.rst:
# FindLibFFI
# -----------
#
# Read-Only variables:
#
# ::
#
#   LIBFFI_FOUND - system has the OpenSSL library
#   LIBFFI_INCLUDE_DIR - the OpenSSL include directory
#   LIBFFI_LIBRARIES - The libraries needed to use OpenSSL
#   LIBFFI_VERSION - This is set to $major.$minor.$revision

#=============================================================================
# Copyright 2014-2014 Weng Xuetian <wengxt@gmail.com>
#

find_package(PkgConfig QUIET)
pkg_check_modules(PC_LIBFFI QUIET libffi)


find_path(LIBFFI_INCLUDE_DIR
  NAMES
    ffi.h
  HINTS
    ${PC_LIBFFI_INCLUDEDIR}
    ${PC_LIBFFI_INCLUDE_DIRS}
)

find_library(LIBFFI_LIBRARY
  NAMES
    ffi
  HINTS
    ${PC_LIBFFI_LIBDIR}
    ${PC_LIBFFI_LIBRARY_DIRS}
)

mark_as_advanced(LIBFFI_LIBRARY)

set(LIBFFI_LIBRARIES ${LIBFFI_LIBRARY})

if (LIBFFI_INCLUDE_DIR)
  if (PC_LIBFFI_VERSION)
    set(LIBFFI_VERSION "${PC_LIBFFI_VERSION}")
  endif ()
endif ()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(LIBFFI
  REQUIRED_VARS
    LIBFFI_LIBRARIES
    LIBFFI_INCLUDE_DIR
  VERSION_VAR
    LIBFFI_VERSION
  FAIL_MESSAGE
    "Could NOT find libffi"
)

mark_as_advanced(LIBFFI_INCLUDE_DIR LIBFFI_LIBRARIES)
