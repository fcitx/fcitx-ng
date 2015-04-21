if (NOT TARGET DBus::DBus)
return()
endif()

fcitx5_generate_addon_function(fcitx-dbus.fxaddon)

add_library(fcitx-dbus MODULE dbusmodule.c
    ${CMAKE_CURRENT_BINARY_DIR}/fcitx-dbus-internal.h
    ${CMAKE_CURRENT_BINARY_DIR}/fcitx-dbus.c
    )
target_include_directories(fcitx-dbus PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(fcitx-dbus DBus::DBus Fcitx::Core)
set_target_properties(fcitx-dbus PROPERTIES PREFIX "")
install(TARGETS fcitx-dbus DESTINATION ${FCITX_INSTALL_ADDONDIR})
fcitx5_install_addon_config(fcitx-dbus)

add_library(fcitx-dbus-interface INTERFACE)
set_target_properties(fcitx-dbus-interface PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_BINARY_DIR}"
)
