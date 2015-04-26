add_executable(fcitx-text2cstring main.c)
set_target_properties(fcitx-text2cstring PROPERTIES
                      OUTPUT_NAME fcitx-text2cstring)
add_executable(Fcitx5::text2cstring ALIAS fcitx-text2cstring)
