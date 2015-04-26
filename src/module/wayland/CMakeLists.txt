if (NOT TARGET Wayland::Client)
return()
endif()

fcitx5_generate_addon_function(fcitx-wayland.fxaddon)

add_library(fcitx-wayland MODULE
    wayland-input.c
    wayland-global.c
    waylandmodule.c
    ${CMAKE_CURRENT_BINARY_DIR}/fcitx-wayland.c
)
target_include_directories(fcitx-wayland PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(fcitx-wayland Fcitx5::Core Wayland::Client)
set_target_properties(fcitx-wayland PROPERTIES PREFIX "")
install(TARGETS fcitx-wayland DESTINATION ${FCITX_INSTALL_ADDONDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/fcitx-wayland.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/fcitx/module/wayland)
fcitx5_install_addon_config(fcitx-wayland)

add_library(fcitx-wayland-interface INTERFACE)
set_target_properties(fcitx-wayland-interface PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_BINARY_DIR}"
)
