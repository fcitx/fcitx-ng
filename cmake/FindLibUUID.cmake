# - Try to find LibUUID
# Once done, this will define
#
#   LIBUUID_FOUND - System has LibUUID
#   LIBUUID_INCLUDE_DIRS - The LibUUID include directories
#   LIBUUID_LIBRARIES - The libraries needed to use LibUUID
#   LIBUUID_DEFINITIONS - Compiler switches required for using LibUUID

find_package(PkgConfig)
pkg_check_modules(PC_LIBUUID QUIET "uuid")
set(LIBUUID_DEFINITIONS ${PC_LIBUUID_CFLAGS_OTHER})

find_path(LIBUUID_INCLUDE_DIR
    NAMES uuid/uuid.h
    HINTS ${PC_LIBUUID_INCLUDE_DIR} ${PC_LIBUUID_INCLUDE_DIRS}
)

find_library(LIBUUID_LIBRARY
    NAMES uuid
    HINTS ${PC_LIBUUID_LIBRARY} ${PC_LIBUUID_LIBRARY_DIRS}
)

set(LIBUUID_LIBRARIES ${LIBUUID_LIBRARY})
set(LIBUUID_INCLUDE_DIRS ${LIBUUID_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibUUID DEFAULT_MSG
    LIBUUID_LIBRARY
    LIBUUID_INCLUDE_DIR
)

mark_as_advanced(LIBUUID_LIBRARY LIBUUID_INCLUDE_DIR)

