include_directories (
  ${LIBINTL_INCLUDE_DIR}
  ${LIBICONV_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${XKBCOMMON_INCLUDE_DIRS}
  )
set(FCITX_KEYBOARD_LINK_LIBS)
if(ENABLE_LIBXML2)
  set(FCITX_KEYBOARD_LINK_LIBS XKBCommon::XKBCommon LibXml2::LibXml)
endif()

set(FCITX_KEYBOARD_SOURCES
  keyboard.c
  ${CMAKE_CURRENT_BINARY_DIR}/keyboard-config.c
  ${CMAKE_CURRENT_BINARY_DIR}/keyboard-config.h
  )
fcitx5_generate_config_source(${CMAKE_CURRENT_SOURCE_DIR}/fcitx-keyboard.desc keyboard-config KeyboardConfig Fcitx)
if(ENABLE_LIBXML2)
  set(FCITX_KEYBOARD_SOURCES ${FCITX_KEYBOARD_SOURCES} isocodes.c rules.c)
endif()

add_library(fcitx-keyboard MODULE ${FCITX_KEYBOARD_SOURCES})
target_link_libraries(fcitx-keyboard ${FCITX_KEYBOARD_LINK_LIBS} Fcitx::Core)
set_target_properties(fcitx-keyboard PROPERTIES PREFIX "")
