find_package(PkgConfig)

pkg_check_modules(PKG_GOBJECT2 QUIET gobject-2.0)

set(GOBJECT2_DEFINITIONS ${PKG_GOBJECT2_CFLAGS_OTHER})
set(GOBJECT2_VERSION ${PKG_GOBJECT2_VERSION})

find_path(GOBJECT2_INCLUDE_DIR
    NAMES glib-object.h
    HINTS ${PKG_GOBJECT2_INCLUDE_DIRS}
)

find_library(GOBJECT2_LIBRARY
    NAMES gobject-2.0
    HINTS ${PKG_GOBJECT2_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GObject2
    FOUND_VAR
        GOBJECT2_FOUND
    REQUIRED_VARS
        GOBJECT2_LIBRARY
        GOBJECT2_INCLUDE_DIR
    VERSION_VAR
        GOBJECT2_VERSION
)

if(GOBJECT2_FOUND AND NOT TARGET GObject2::GObject)
    add_library(GObject2::GObject UNKNOWN IMPORTED)
    set_target_properties(GObject2::GObject PROPERTIES
        IMPORTED_LOCATION "${GOBJECT2_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${GOBJECT2_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${GOBJECT2_INCLUDE_DIR}"
    )
endif()

mark_as_advanced(GOBJECT2_INCLUDE_DIR GOBJECT2_LIBRARY)

include(FeatureSummary)
set_package_properties(GOBJECT2 PROPERTIES
    URL "http://www.gtk.org/"
    DESCRIPTION "Common C routines used by GTK+ and other libs"
)
