find_package(PkgConfig)

pkg_check_modules(PKG_GLIB2 QUIET glib-2.0)

set(GLIB2_DEFINITIONS ${PKG_GLIB2_CFLAGS_OTHER})
set(GLIB2_VERSION ${PKG_GLIB2_VERSION})

find_path(GLIB2_INCLUDE_DIR
    NAMES glib.h
    HINTS ${PKG_GLIB2_INCLUDE_DIRS}
)

find_path(GLIB2_ARCH_INCLUDE_DIR
    NAMES glibconfig.h
    HINTS ${PKG_GLIB2_INCLUDE_DIRS}
)

find_library(GLIB2_LIBRARY
    NAMES glib-2.0
    HINTS ${PKG_GLIB2_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLib2
    FOUND_VAR
        GLIB2_FOUND
    REQUIRED_VARS
        GLIB2_LIBRARY
        GLIB2_INCLUDE_DIR
        GLIB2_ARCH_INCLUDE_DIR
    VERSION_VAR
        GLIB2_VERSION
)

if(GLIB2_FOUND AND NOT TARGET GLib2::GLib)
    add_library(GLib2::GLib UNKNOWN IMPORTED)
    set_target_properties(GLib2::GLib PROPERTIES
        IMPORTED_LOCATION "${GLIB2_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${GLIB2_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${GLIB2_INCLUDE_DIR};${GLIB2_ARCH_INCLUDE_DIR}"
    )
endif()

mark_as_advanced(GLIB2_INCLUDE_DIR GLIB2_ARCH_INCLUDE_DIR GLIB2_LIBRARY)

include(FeatureSummary)
set_package_properties(GLIB2 PROPERTIES
    URL "http://www.gtk.org/"
    DESCRIPTION "Common C routines used by GTK+ and other libs"
)
