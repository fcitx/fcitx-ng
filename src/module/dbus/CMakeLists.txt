include_directories(${DBUS_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

fcitx5_generate_addon_function(fcitx-dbus.fxaddon)

add_library(fcitx-dbus MODULE dbusmodule.c
    ${CMAKE_CURRENT_BINARY_DIR}/fcitx-dbus-internal.h
    ${CMAKE_CURRENT_BINARY_DIR}/fcitx-dbus.c
    )
target_link_libraries(fcitx-dbus ${DBUS_LIBRARIES} Fcitx::Core)
set_target_properties(fcitx-dbus PROPERTIES PREFIX "")
install(TARGETS fcitx-dbus DESTINATION ${FCITX_INSTALL_ADDONDIR})
fcitx5_install_addon_config(fcitx-dbus)
