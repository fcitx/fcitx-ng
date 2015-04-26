set(FCITX_CONFIG_SOURCES
    configuration.c
    iniparser.c
    helper.c
    description.c
    )

set(FCITX_CONFIG_HEADERS
    configuration.h
    iniparser.h
    helper.h
    description.h
    )


add_library(fcitx-config SHARED ${FCITX_CONFIG_SOURCES})
set_target_properties(fcitx-config
  PROPERTIES VERSION 5.0
  SOVERSION 5
  COMPILE_FLAGS "-fvisibility=hidden"
  LINK_FLAGS "-Wl,--no-undefined"
  EXPORT_NAME Config
  )
target_link_libraries(fcitx-config Fcitx5::Utils)
target_include_directories(fcitx-config PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_FULL_INCLUDEDIR}>/Fcitx5/FcitxConfig")

configure_file(fcitx-config.pc.in ${CMAKE_CURRENT_BINARY_DIR}/fcitx-config.pc @ONLY)

install(TARGETS fcitx-config EXPORT FcitxConfigTargets LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/fcitx-config.pc
  DESTINATION "${CMAKE_INSTALL_LIBDATADIR}/pkgconfig")
install(FILES ${FCITX_CONFIG_HEADERS} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/Fcitx5/FcitxConfig/fcitx-config")

add_library(Fcitx5::Config ALIAS fcitx-config)

ecm_setup_version(PROJECT
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/FcitxConfigConfigVersion.cmake"
                  SOVERSION 5)

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/FcitxConfigConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/FcitxConfigConfig.cmake"
                              INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/FcitxConfig
)

install(EXPORT FcitxConfigTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/FcitxConfig FILE FcitxConfigTargets.cmake NAMESPACE Fcitx5::)

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/FcitxConfigConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/FcitxConfigConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/FcitxConfig
        COMPONENT Devel )
