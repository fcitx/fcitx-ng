fcitx5_merge_config_translation(keyboard.conf.in ${CMAKE_CURRENT_BINARY_DIR}/keyboard.conf)
fcitx5_generate_binary_file_header(${CMAKE_CURRENT_BINARY_DIR}/keyboard.conf keyboard_conf.h keyboard_conf)
set(FCITX_KEYBOARD_SOURCES
  keyboard.c
  ${CMAKE_CURRENT_BINARY_DIR}/keyboard-config.c
  keyboard_conf.h
  )
fcitx5_generate_config_source(${CMAKE_CURRENT_SOURCE_DIR}/fcitx-keyboard.desc keyboard-config KeyboardConfig Fcitx)
if(ENABLE_LIBXML2)
  set(FCITX_KEYBOARD_SOURCES ${FCITX_KEYBOARD_SOURCES} isocodes.c rules.c)
endif()

add_library(fcitx-keyboard MODULE ${FCITX_KEYBOARD_SOURCES})
target_include_directories(fcitx-keyboard PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(fcitx-keyboard XKBCommon::XKBCommon LibXml2::LibXml Fcitx5::Core fcitx-xcb-interface)
set_target_properties(fcitx-keyboard PROPERTIES PREFIX "")
