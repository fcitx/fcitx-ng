include_directories(${DBUS_INCLUDE_DIRS})

add_library(fcitx-ipc MODULE ipc.c)
add_dependencies(fcitx-ipc fcitx-dbus.h.target)
target_link_libraries(fcitx-ipc ${DBUS_LIBRARIES} Fcitx::Core)
install(TARGETS fcitx-ipc DESTINATION ${FCITX_INSTALL_ADDONDIR})
set_target_properties(fcitx-ipc PROPERTIES PREFIX "")
fcitx5_install_addon_config(fcitx-ipc)
