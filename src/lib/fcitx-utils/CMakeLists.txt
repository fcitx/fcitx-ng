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
    utils.h
    sort_common.h
    )

add_library(fcitx-utils SHARED ${FCITX_UTILS_SOURCES})
set_target_properties(fcitx-utils
  PROPERTIES VERSION 1.0
  SOVERSION 1
  COMPILE_FLAGS "-fvisibility=hidden"
  LINK_FLAGS "-Wl,--no-undefined"
  EXPORT_NAME Utils
  )
target_include_directories(fcitx-utils PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_FULL_INCLUDEDIR}>)
target_link_libraries(fcitx-utils
    PRIVATE
        DL::DL RT::RT)

configure_file(fcitx-utils.pc.in ${CMAKE_CURRENT_BINARY_DIR}/fcitx-utils.pc)

install(TARGETS fcitx-utils EXPORT FcitxUtilsTargets LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/fcitx-utils.pc
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
install(FILES ${FCITX_UTILS_HEADERS} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/fcitx-utils")

add_library(Fcitx::Utils ALIAS fcitx-utils)

ecm_setup_version(PROJECT
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/FcitxUtilsConfigVersion.cmake"
                  SOVERSION 1)

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/FcitxUtilsConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/FcitxUtilsConfig.cmake"
                              INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/FcitxUtils
)

install(EXPORT FcitxUtilsTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/FcitxUtils FILE FcitxUtilsTargets.cmake NAMESPACE Fcitx::)

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/FcitxUtilsConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/FcitxUtilsConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/FcitxUtils
        COMPONENT Devel )
