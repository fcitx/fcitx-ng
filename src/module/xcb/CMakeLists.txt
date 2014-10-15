include_directories(${XCB_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

fcitx5_generate_addon_function(fcitx-xcb.fxaddon)

add_library(fcitx-xcb MODULE
    xcbmodule.c
    ${CMAKE_CURRENT_BINARY_DIR}/fcitx-xcb-internal.h
    ${CMAKE_CURRENT_BINARY_DIR}/fcitx-xcb.c
)
target_link_libraries(fcitx-xcb Fcitx::Core XCB::XCB)
set_target_properties(fcitx-xcb PROPERTIES PREFIX "")
install(TARGETS fcitx-xcb DESTINATION ${FCITX_INSTALL_ADDONDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/fcitx-xcb.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/fcitx/module/xcb)
fcitx5_install_addon_config(fcitx-xcb)
