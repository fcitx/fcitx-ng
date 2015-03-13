find_package(PkgConfig)

pkg_check_modules(PKG_DBUS QUIET dbus-1)

set(DBUS_DEFINITIONS ${PKG_DBUS_CFLAGS_OTHER})
set(DBUS_VERSION ${PKG_DBUS_VERSION})

find_path(DBUS_INCLUDE_DIR
    NAMES dbus/dbus.h
    HINTS ${PKG_DBUS_INCLUDE_DIRS}
)

find_path(DBUS_ARCH_INCLUDE_DIR
    NAMES dbus/dbus-arch-deps.h
    HINTS ${PKG_DBUS_INCLUDE_DIRS}
)

find_library(DBUS_LIBRARY
    NAMES dbus-1
    HINTS ${PKG_DBUS_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(DBus
    FOUND_VAR
        DBUS_FOUND
    REQUIRED_VARS
        DBUS_LIBRARY
        DBUS_INCLUDE_DIR
        DBUS_ARCH_INCLUDE_DIR
    VERSION_VAR
        DBUS_VERSION
)

if(DBUS_FOUND AND NOT TARGET DBus::DBus)
    add_library(DBus::DBus UNKNOWN IMPORTED)
    set_target_properties(DBus::DBus PROPERTIES
        IMPORTED_LOCATION "${DBUS_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${DBUS_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${DBUS_INCLUDE_DIR};${DBUS_ARCH_INCLUDE_DIR}"
    )
endif()

mark_as_advanced(DBUS_INCLUDE_DIR DBUS_ARCH_INCLUDE_DIR DBUS_LIBRARY)

include(FeatureSummary)
set_package_properties(DBUS PROPERTIES
    URL "http://www.freedesktop.org/Software/dbus"
    DESCRIPTION "Freedesktop.org message bus system"
)
