set(FCITX_GCLIENT_SOURCES
  fcitxclient.c
  fcitxinputmethod.c
  fcitxkbd.c
  fcitxconnection.c
  )

set(FCITX_GCLIENT_BUILT_SOURCES
  ${CMAKE_CURRENT_BINARY_DIR}/marshall.c
  )

set(FCITX_GCLIENT_HEADERS
  fcitxclient.h
  fcitxinputmethod.h
  fcitxkbd.h
  fcitxconnection.h
  )

set(FCITX_GCLIENT_BUILT_HEADERS
  ${CMAKE_CURRENT_BINARY_DIR}/marshall.h
  )

pkg_check_variable(GLIB2_GLIB_GENMARSHAL "glib-2.0" "glib_genmarshal")
find_program(GLIB_GENMARSHAL ${GLIB2_GLIB_GENMARSHAL})

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_custom_command(OUTPUT marshall.c
  COMMAND ${GLIB_GENMARSHAL} --body --prefix=fcitx_marshall
  ${CMAKE_CURRENT_SOURCE_DIR}/marshall.list > marshall.c
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/marshall.list)
add_custom_command(OUTPUT marshall.h
  COMMAND ${GLIB_GENMARSHAL} --header --prefix=fcitx_marshall
  ${CMAKE_CURRENT_SOURCE_DIR}/marshall.list > marshall.h
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/marshall.list)

add_library(fcitx-gclient SHARED ${FCITX_GCLIENT_SOURCES}
  ${FCITX_GCLIENT_BUILT_SOURCES} ${FCITX_GCLIENT_BUILT_HEADERS})
set_target_properties(fcitx-gclient
  PROPERTIES VERSION 0.1
  SOVERSION 1
  COMPILE_FLAGS "-fvisibility=hidden"
  LINK_FLAGS "-Wl,--no-undefined")
target_link_libraries(fcitx-gclient GLib2::GLib GObject2::GObject Gio2::Gio Fcitx::Utils)
configure_file(fcitx-gclient.pc.in ${CMAKE_CURRENT_BINARY_DIR}/fcitx-gclient.pc)

install(TARGETS fcitx-gclient LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/fcitx-gclient.pc
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
install(FILES ${FCITX_GCLIENT_HEADERS}
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/fcitx-gclient")

if(ENABLE_GIR)
  include(GObjectIntrospection)
  gobject_introspection(
    FILENAME Fcitx-1.0.gir
    NSVERSION 1.0
    INCLUDE Gio-2.0 GObject-2.0 GLib-2.0
    PACKAGE_EXPORT fcitx-gclient
    LIBRARY fcitx-gclient
    SCANNER_ARGS --warn-all --add-include-path=${CMAKE_CURRENT_SOURCE_DIR}
    COMPILER_ARGS "--includedir=${CMAKE_CURRENT_SOURCE_DIR}"
    SYMBOL_PREFIXES fcitx
    SOURCES ${FCITX_GCLIENT_SOURCES} ${FCITX_GCLIENT_HEADERS}
    QUIET
    )
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/Fcitx-1.0.gir"
    DESTINATION "${GOBJECT_INTROSPECTION_GIRDIR}")
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/Fcitx-1.0.typelib"
    DESTINATION "${GOBJECT_INTROSPECTION_TYPELIBDIR}")
endif()
