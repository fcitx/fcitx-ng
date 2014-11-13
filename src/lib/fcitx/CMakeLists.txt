include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${LIBFFI_INCLUDE_DIR}
                    ${LIBUUID_INCLUDE_DIR})

set(LIBFCITX_SOURCES
  instance.c
  ${CMAKE_CURRENT_BINARY_DIR}/addon-config.c
  ${CMAKE_CURRENT_BINARY_DIR}/addon-config.h
  addon.c
  inputcontext.c
  inputdevice.c
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

target_link_libraries(fcitx-core Fcitx::Config Fcitx::Utils ${LIBFFI_LIBRARIES} ${LIBUUID_LIBRARIES}
  ${DL_LIBRARIES} ${PTHREAD_LIBRARIES})
install(TARGETS fcitx-core LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/fcitx.pc
  DESTINATION "${CMAKE_INSTALL_LIBDATADIR}/pkgconfig")
install(FILES ${FCITX_HEADERS} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/fcitx")

add_library(Fcitx::Core ALIAS fcitx-core)
