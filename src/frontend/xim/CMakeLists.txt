if (NOT TARGET XCBImdkit::XCBImdkit AND
    NOT TARGET fcitx-xcb-interface AND
    NOT TARGET XCB::XCB AND
    NOT TARGET XCB::AUX AND
    NOT TARGET XKBCommon::XKBCommon)
return()
endif()

fcitx5_generate_config_source(fcitx-xim.desc xim-config XIMConfig Fcitx)

add_library(fcitx-xim MODULE xim.c ${CMAKE_CURRENT_BINARY_DIR}/xim-config.c)
target_include_directories(fcitx-xim PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(fcitx-xim Fcitx::Core fcitx-xcb-interface XCBImdkit::XCBImdkit XCB::XCB XCB::AUX XKBCommon::XKBCommon)
install(TARGETS fcitx-xim DESTINATION ${FCITX_INSTALL_ADDONDIR})
add_dependencies(fcitx-xim fcitx-xcb.h.target)
set_target_properties(fcitx-xim PROPERTIES PREFIX "")
fcitx5_install_addon_config(fcitx-xim)
