set(FCITX_IM_SOURCES
    pinyin/pinyinbasedict.c
    table/tabledict.c
    )

set(FCITX_IM_HEADERS
    )

add_library(fcitx-im SHARED ${FCITX_IM_SOURCES})
set_target_properties(fcitx-im
  PROPERTIES VERSION 1.0
  SOVERSION 1
  COMPILE_FLAGS "-fvisibility=hidden"
  LINK_FLAGS "-Wl,--no-undefined"
  )
target_link_libraries(fcitx-im fcitx-utils)
