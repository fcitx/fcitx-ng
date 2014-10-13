include_directories(${DBUS_INCLUDE_DIRS})
add_library(fcitx-dbus MODULE dbusmodule.c)
target_link_libraries(fcitx-dbus ${DBUS_LIBRARIES} Fcitx::Core)
install(TARGETS fcitx-dbus DESTINATION ${FCITX_INSTALL_ADDONDIR})
