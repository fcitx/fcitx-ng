add_executable(fcitx-configdesc-compiler main.c header.c pot.c source.c ../common/common.c)
set_target_properties(fcitx-configdesc-compiler PROPERTIES
                      OUTPUT_NAME fcitx-configdesc-compiler)
target_link_libraries(fcitx-configdesc-compiler fcitx-config)
target_include_directories(fcitx-configdesc-compiler PUBLIC ../common/)
add_executable(Fcitx5::configdesc-compiler ALIAS fcitx-configdesc-compiler)
