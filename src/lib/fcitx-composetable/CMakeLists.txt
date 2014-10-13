set(FCITX_COMPOSETABLE_SOURCES
    tablegenerator.c
    composecontext.c
    )

set(FCITX_COMPOSETABLE_HEADERS
    tablegenerator.h
    composecontext.h
    )

add_library(fcitx-composetable SHARED ${FCITX_COMPOSETABLE_SOURCES})
set_target_properties(fcitx-composetable
  PROPERTIES VERSION 1.0
  SOVERSION 1
  COMPILE_FLAGS "-fvisibility=hidden"
  LINK_FLAGS "-Wl,--no-undefined"
  )
target_link_libraries(fcitx-composetable Fcitx::Utils)

add_library(Fcitx::ComposeTable ALIAS fcitx-composetable)
