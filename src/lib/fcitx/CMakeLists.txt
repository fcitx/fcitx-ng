set(LIBFCITX_SOURCES
  instance.c
  ${CMAKE_CURRENT_BINARY_DIR}/addon-config.c
  ${CMAKE_CURRENT_BINARY_DIR}/global-config.c
  ${CMAKE_CURRENT_BINARY_DIR}/input-method-metadata.c
  ${CMAKE_CURRENT_BINARY_DIR}/input-method-list.c
  addon.c
  inputcontext.c
  inputdevice.c
  ime.c
  ui.c
  isocodes.c
  rules.c
  )

set(FCITX_HEADERS
    addon.h
    instance.h
    inputcontext.h
    ui.h
    event.h
  )

set(FCITX_INTERNAL_HEADERS
  instance-internal.h
  )

fcitx5_generate_config_source(${PROJECT_SOURCE_DIR}/data/addon.desc addon-config AddonConfig Fcitx)
fcitx5_generate_config_source(${PROJECT_SOURCE_DIR}/data/config.desc global-config GlobalConfig Fcitx)
fcitx5_generate_config_source(${PROJECT_SOURCE_DIR}/data/inputmethod.desc input-method-metadata InputMethodMetadata Fcitx)
fcitx5_generate_config_source(${PROJECT_SOURCE_DIR}/data/inputmethodlist.desc input-method-list InputMethodList Fcitx)

link_directories(${DL_LIBRARY_DIRS} ${PTHREAD_LIBRARY_DIRS})

add_library(fcitx-core SHARED ${LIBFCITX_SOURCES})
set_target_properties(fcitx-core
  PROPERTIES VERSION 1.0
  SOVERSION 1
  COMPILE_FLAGS "-fvisibility=hidden"
  LINK_FLAGS "-Wl,--no-undefined"
  EXPORT_NAME Core)

configure_file(fcitx.pc.in ${CMAKE_CURRENT_BINARY_DIR}/fcitx.pc @ONLY)

target_link_libraries(fcitx-core
    PUBLIC
        Fcitx5::Config Fcitx5::Utils
    PRIVATE
        LibFFI::FFI LibUUID::UUID DL::DL LibXml2::LibXml)
target_include_directories(fcitx-core PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_FULL_INCLUDEDIR}>/Fcitx5/FcitxCore")
install(TARGETS fcitx-core EXPORT FcitxCoreTargets LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/fcitx.pc
  DESTINATION "${CMAKE_INSTALL_LIBDATADIR}/pkgconfig")
install(FILES ${FCITX_HEADERS} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/Fcitx5/FcitxCore/fcitx")

add_library(Fcitx5::Core ALIAS fcitx-core)

ecm_setup_version(PROJECT
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/FcitxCoreConfigVersion.cmake"
                  SOVERSION 1)

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/FcitxCoreConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/FcitxCoreConfig.cmake"
                              INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/FcitxCore
)

install(EXPORT FcitxCoreTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/FcitxCore FILE FcitxCoreTargets.cmake NAMESPACE Fcitx5::)

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/FcitxCoreConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/FcitxCoreConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/FcitxCore
        COMPONENT Devel )
