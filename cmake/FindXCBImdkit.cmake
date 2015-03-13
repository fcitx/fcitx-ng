find_package(PkgConfig)

pkg_check_modules(PKG_XCBIMDKIT QUIET xcb-imdkit)

set(XCBIMDKIT_DEFINITIONS ${PKG_XCBIMDKIT_CFLAGS_OTHER})
set(XCBIMDKIT_VERSION ${PKG_XCBIMDKIT_VERSION})

find_path(XCBIMDKIT_INCLUDE_DIR
    NAMES xcb-imdkit/imdkit.h
    HINTS ${PKG_XCBIMDKIT_INCLUDE_DIRS}
)
find_library(XCBIMDKIT_LIBRARY
    NAMES xcb-imdkit
    HINTS ${PKG_XCBIMDKIT_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(XCBImdkit
    FOUND_VAR
        XCBIMDKIT_FOUND
    REQUIRED_VARS
        XCBIMDKIT_LIBRARY
        XCBIMDKIT_INCLUDE_DIR
    VERSION_VAR
        XCBIMDKIT_VERSION
)

if(XCBIMDKIT_FOUND AND NOT TARGET XCBImdkit::XCBImdkit)
    add_library(XCBImdkit::XCBImdkit UNKNOWN IMPORTED)
    set_target_properties(XCBImdkit::XCBImdkit PROPERTIES
        IMPORTED_LOCATION "${XCBIMDKIT_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${XCBIMDKIT_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${XCBIMDKIT_INCLUDE_DIR}"
    )
endif()

mark_as_advanced(XCBIMDKIT_INCLUDE_DIR XCBIMDKIT_LIBRARY)

include(FeatureSummary)
set_package_properties(XCBIMDKIT PROPERTIES
    URL "https://github.com/wengxt/xcb-imdkit"
    DESCRIPTION "XCB based imdkit"
)
