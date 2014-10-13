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
  PROPERTIES VERSION 1.0
  SOVERSION 1
  COMPILE_FLAGS "-fvisibility=hidden"
  LINK_FLAGS "-Wl,--no-undefined"
  )
target_link_libraries(fcitx-config Fcitx::Utils)

add_library(Fcitx::Config ALIAS fcitx-config)
