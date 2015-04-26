if (NOT TARGET XCB::XCB OR
    NOT TARGET XCB::AUX OR
    NOT TARGET XCB::XKB OR
    NOT TARGET XKBCommon::XKBCommon OR
    NOT TARGET XKBCommon::X11)
return()
endif()

fcitx5_generate_addon_function(fcitx-xcb.fxaddon)

add_library(fcitx-xcb MODULE
    xcbmodule.c
    ${CMAKE_CURRENT_BINARY_DIR}/fcitx-xcb.c
)
target_include_directories(fcitx-xcb PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(fcitx-xcb Fcitx5::Core XCB::XCB XCB::AUX XCB::XKB XKBCommon::XKBCommon XKBCommon::X11)
set_target_properties(fcitx-xcb PROPERTIES PREFIX "")
install(TARGETS fcitx-xcb DESTINATION ${FCITX_INSTALL_ADDONDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/fcitx-xcb.h DESTINATION "${FCITX_INSTALL_MODULE_HEADER_DIR}")
fcitx5_install_addon_config(fcitx-xcb)

add_library(fcitx-xcb-interface INTERFACE)
set_target_properties(fcitx-xcb-interface PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_BINARY_DIR}"
)
