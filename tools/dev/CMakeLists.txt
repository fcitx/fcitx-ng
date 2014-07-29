add_executable(fcitx-configdesc-compiler config-compiler.c)
set_target_properties(fcitx-configdesc-compiler PROPERTIES
                      OUTPUT_NAME fcitx-configdesc-compiler)
target_link_libraries(fcitx-configdesc-compiler fcitx-config)
add_executable(Fcitx::configdesc-compiler ALIAS fcitx-configdesc-compiler)
