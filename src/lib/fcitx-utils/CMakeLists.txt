set(FCITX_UTILS_SOURCES
    key.c
    utf8.c
    qsort_r.c
    msort_r.c
    utils.c
    handler-table.c
    memory-pool.c
    objpool.c
    stringmap.c
    stringutils.c
    stringlist.c
    stringhashset.c
    dict.c
    xdg.c
    fs.c
    )

set(FCITX_UTILS_HEADERS
    key.h
    utf8.h
    keydata.h
    keysym.h
    keysymdef.h
    uthash.h
    utarray.h
    utils.h
    handler-table.h
    memory-pool.h
    objpool.h
    stringmap.h
    stringutils.h
    stringlist.h
    stringhashset.h
    dict.h
    xdg.h
    fs.h
    )

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
target_link_libraries(fcitx-utils ${LIBINTL_LIBRARIES}
  ${LIBICONV_LIBRARIES} ${LIBEXECINFO_LIBRARIES})

if(LIBKVM_FOUND)
  target_link_libraries(fcitx-utils ${LIBKVM_LIBRARIES})
endif()

configure_file(fcitx-utils.pc.in ${CMAKE_CURRENT_BINARY_DIR}/fcitx-utils.pc)

install(TARGETS fcitx-utils LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/fcitx-utils.pc
  DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(FILES ${FCITX_UTILS_HEADERS} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/fcitx-utils")

if(ENABLE_STATIC)
  add_library(fcitx-utils-static STATIC ${FCITX_UTILS_SOURCES})
  set_target_properties(fcitx-utils-static PROPERTIES
    OUTPUT_NAME "fcitx-utils"
    PREFIX "lib")
  install(TARGETS fcitx-utils-static
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
