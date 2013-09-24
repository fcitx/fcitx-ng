set(LIBFCITX_SOURCES
  instance.c
  addon.c
  )

set(FCITX_HEADERS
  instance.h
  addon.h
  )

set(FCITX_INTERNAL_HEADERS
  instance-internal.h
  )

link_directories(${DL_LIBRARY_DIRS} ${PTHREAD_LIBRARY_DIRS})

add_library(fcitx-core SHARED ${LIBFCITX_SOURCES})
set_target_properties(fcitx-core
  PROPERTIES VERSION 1.0
  SOVERSION 1
  COMPILE_FLAGS "-fvisibility=hidden"
  LINK_FLAGS "-Wl,--no-undefined")

configure_file(fcitx.pc.in ${CMAKE_CURRENT_BINARY_DIR}/fcitx.pc)

target_link_libraries(fcitx-core fcitx-config fcitx-utils
  ${DL_LIBRARIES} ${PTHREAD_LIBRARIES})
install(TARGETS fcitx-core LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/fcitx.pc
  DESTINATION "${CMAKE_INSTALL_LIBDATADIR}/pkgconfig")
install(FILES ${FCITX_HEADERS} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/fcitx")

if(ENABLE_STATIC)
  add_library(fcitx-core-static STATIC ${LIBFCITX_SOURCES})
  set_target_properties(fcitx-core-static PROPERTIES
    OUTPUT_NAME "fcitx-core"
    PREFIX "lib")

  install(TARGETS fcitx-core-static
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}")
endif()
