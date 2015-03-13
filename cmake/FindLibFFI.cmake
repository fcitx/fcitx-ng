find_package(PkgConfig)

pkg_check_modules(PKG_LIBFFI QUIET libffi)

set(LIBFFI_DEFINITIONS ${PKG_LIBFFI_CFLAGS_OTHER})
set(LIBFFI_VERSION ${PKG_LIBFFI_VERSION})

find_path(LIBFFI_INCLUDE_DIR
    NAMES ffi.h
    HINTS ${PKG_LIBFFI_INCLUDE_DIRS}
)
find_library(LIBFFI_LIBRARY
    NAMES ffi
    HINTS ${PKG_LIBFFI_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibFFI
    FOUND_VAR
        LIBFFI_FOUND
    REQUIRED_VARS
        LIBFFI_LIBRARY
        LIBFFI_INCLUDE_DIR
    VERSION_VAR
        LIBFFI_VERSION
)

if(LIBFFI_FOUND AND NOT TARGET LibFFI::FFI)
    add_library(LibFFI::FFI UNKNOWN IMPORTED)
    set_target_properties(LibFFI::FFI PROPERTIES
        IMPORTED_LOCATION "${LIBFFI_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${LIBFFI_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${LIBFFI_INCLUDE_DIR}"
    )
endif()

mark_as_advanced(LIBFFI_INCLUDE_DIR LIBFFI_LIBRARY)

# compatibility variables
set(LIBFFI_LIBRARIES ${LIBFFI_LIBRARY})
set(LIBFFI_INCLUDE_DIRS ${LIBFFI_INCLUDE_DIR})
set(LIBFFI_VERSION_STRING ${LIBFFI_VERSION})

include(FeatureSummary)
set_package_properties(LIBFFI PROPERTIES
    URL "http://sourceware.org/libffi"
    DESCRIPTION "A portable, high level programming interface to various calling conventions."
)
