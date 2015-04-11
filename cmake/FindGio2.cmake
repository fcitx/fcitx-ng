find_package(PkgConfig)

pkg_check_modules(PKG_GIO2 QUIET glib-2.0)

set(GIO2_DEFINITIONS ${PKG_GIO2_CFLAGS_OTHER})
set(GIO2_VERSION ${PKG_GIO2_VERSION})

find_path(GIO2_INCLUDE_DIR
    NAMES gio/gio.h
    HINTS ${PKG_GIO2_INCLUDE_DIRS}
)

find_library(GIO2_LIBRARY
    NAMES gio-2.0
    HINTS ${PKG_GIO2_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Gio2
    FOUND_VAR
        GIO2_FOUND
    REQUIRED_VARS
        GIO2_LIBRARY
        GIO2_INCLUDE_DIR
    VERSION_VAR
        GIO2_VERSION
)

if(GIO2_FOUND AND NOT TARGET Gio2::Gio)
    add_library(Gio2::Gio UNKNOWN IMPORTED)
    set_target_properties(Gio2::Gio PROPERTIES
        IMPORTED_LOCATION "${GIO2_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${GIO2_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${GIO2_INCLUDE_DIR}"
    )
endif()

mark_as_advanced(GIO2_INCLUDE_DIR GIO2_LIBRARY)

include(FeatureSummary)
set_package_properties(GIO2 PROPERTIES
    URL "http://www.gtk.org/"
    DESCRIPTION "Common C routines used by GTK+ and other libs"
)
