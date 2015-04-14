set(FCITX_GTK2_IM_MODULE_SOURCES
  fcitxim.c
  fcitximcontext.c
  )

if(NOT ENABLE_GTK2_IM_MODULE)
  return()
endif()

PKG_CHECK_MODULES(GTK2 REQUIRED "gtk+-2.0" )
_pkgconfig_invoke("gtk+-2.0" GTK2 BINARY_VERSION
  "" "--variable=gtk_binary_version")

if (NOT DEFINED GTK2_IM_MODULEDIR)
    set(GTK2_IM_MODULEDIR "${CMAKE_INSTALL_LIBDIR}/gtk-2.0/${GTK2_BINARY_VERSION}/immodules" CACHE PATH "Gtk2 im module directory")
endif()

add_custom_command(OUTPUT marshall.c
  COMMAND ${GLIB_GENMARSHAL} --body --prefix=fcitx_marshall
  ${PROJECT_SOURCE_DIR}/src/frontend/gtk-common/marshall.list > marshall.c
  DEPENDS ${PROJECT_SOURCE_DIR}/src/frontend/gtk-common/marshall.list)
add_custom_command(OUTPUT marshall.h
  COMMAND ${GLIB_GENMARSHAL} --header --prefix=fcitx_marshall
  ${PROJECT_SOURCE_DIR}/src/frontend/gtk-common/marshall.list > marshall.h
  DEPENDS ${PROJECT_SOURCE_DIR}/src/frontend/gtk-common/marshall.list)

add_library(im-fcitx MODULE ${FCITX_GTK2_IM_MODULE_SOURCES})
set_target_properties( im-fcitx PROPERTIES PREFIX ""
  COMPILE_FLAGS "-fvisibility=hidden" LINK_FLAGS "-Wl,--no-undefined")
target_link_libraries(im-fcitx GTK2::gtk Fcitx::Utils Fcitx::GClient)
install(TARGETS im-fcitx DESTINATION "${GTK2_IM_MODULEDIR}")
