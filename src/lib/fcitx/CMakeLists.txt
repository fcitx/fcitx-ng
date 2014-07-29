include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(LIBFCITX_SOURCES
  ${CMAKE_CURRENT_BINARY_DIR}/addon-config.c
  ${CMAKE_CURRENT_BINARY_DIR}/addon-config.h
  addon.c
  instance.c
  )

set(FCITX_HEADERS
    addon.h
    instance.h
  )

set(FCITX_INTERNAL_HEADERS
  instance-internal.h
  )

fcitx5_generate_config_source(${PROJECT_SOURCE_DIR}/data/addon.desc addon-config AddonConfig Fcitx)

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
