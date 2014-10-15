include_directories(${CMAKE_CURRENT_BINARY_DIR})

fcitx5_generate_config_source(${CMAKE_CURRENT_SOURCE_DIR}/fxaddon.desc fxaddon FcitxFunctionInfo "")
fcitx5_generate_config_source(${CMAKE_CURRENT_SOURCE_DIR}/fxfunction.desc fxfunction FcitxFunctionSignatureInfo "")
add_executable(fcitx-addon-function-compiler
    main.c
    ../common/common.c
    ${CMAKE_CURRENT_BINARY_DIR}/fxaddon.c
    ${CMAKE_CURRENT_BINARY_DIR}/fxaddon.h
    ${CMAKE_CURRENT_BINARY_DIR}/fxfunction.c
    ${CMAKE_CURRENT_BINARY_DIR}/fxfunction.h
)
set_target_properties(fcitx-addon-function-compiler PROPERTIES
                      OUTPUT_NAME fcitx-addon-function-compiler)
target_link_libraries(fcitx-addon-function-compiler Fcitx::Config)
add_executable(Fcitx::addon-function-compiler ALIAS fcitx-addon-function-compiler)
