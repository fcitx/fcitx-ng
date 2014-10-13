set(FCITX_UTILS_SOURCES
    key.c
    utf8.c
    qsort_r.c
    msort_r.c
    misc.c
    handler-table.c
    memory-pool.c
    objpool.c
    stringmap.c
    stringutils.c
    stringlist.c
    stringhashset.c
    dict.c
    fs.c
    library.c
    mainloop.c
    list.c
    array.c
    standardpath.c
    i18nstring.c
    color.c
    )

set(FCITX_UTILS_HEADERS
    key.h
    utf8.h
    keydata.h
    keysym.h
    keysymgen.h
    uthash.h
    utarray.h
    misc.h
    handler-table.h
    memory-pool.h
    objpool.h
    stringmap.h
    stringutils.h
    stringlist.h
    stringhashset.h
    dict.h
    fs.h
    library.h
    mainloop.h
    list.h
    macro.h
    bitset.h
    types.h
    atomic.h
    array.h
    standardpath.h
    i18nstring.h
    color.h
    rect.h
    )

include_directories(${DL_INCLUDE_DIR})

if(LIBKVM_FOUND)
  include_directories(LIBKVM_INCLUDE_DIR)
endif()

add_library(fcitx-utils SHARED ${FCITX_UTILS_SOURCES})
set_target_properties(fcitx-utils
  PROPERTIES VERSION 1.0
  SOVERSION 1
  COMPILE_FLAGS "-fvisibility=hidden"
  LINK_FLAGS "-Wl,--no-undefined"
  )
target_link_libraries(fcitx-utils ${LIBINTL_LIBRARIES} ${DL_LIBRARIES} ${RT_LIBRARIES}
  ${LIBICONV_LIBRARIES} ${LIBEXECINFO_LIBRARIES})

if(LIBKVM_FOUND)
  target_link_libraries(fcitx-utils ${LIBKVM_LIBRARIES})
endif()

configure_file(fcitx-utils.pc.in ${CMAKE_CURRENT_BINARY_DIR}/fcitx-utils.pc)

install(TARGETS fcitx-utils LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/fcitx-utils.pc
  DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(FILES ${FCITX_UTILS_HEADERS} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/fcitx-utils")

add_library(Fcitx::Utils ALIAS fcitx-utils)
