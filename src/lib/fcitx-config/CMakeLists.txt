set(FCITX_CONFIG_SOURCES
    configuration.c
    iniparser.c
    helper.c
    description.c
    )

set(FCITX_CONFIG_HEADERS
    configuration.h
    iniparser.h
    helper.h
    description.h
    )


add_library(fcitx-config SHARED ${FCITX_CONFIG_SOURCES})
set_target_properties(fcitx-config
  PROPERTIES VERSION 5.0
  SOVERSION 5
  COMPILE_FLAGS "-fvisibility=hidden"
  LINK_FLAGS "-Wl,--no-undefined"
  )
target_link_libraries(fcitx-config Fcitx::Utils)

configure_file(fcitx-config.pc.in ${CMAKE_CURRENT_BINARY_DIR}/fcitx-config.pc)

install(TARGETS fcitx-config LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/fcitx-config.pc
  DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(FILES ${FCITX_CONFIG_HEADERS} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/fcitx-config")

add_library(Fcitx::Config ALIAS fcitx-config)
