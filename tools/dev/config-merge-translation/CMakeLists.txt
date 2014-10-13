include_directories(${GETTEXTPO_INCLUDE_DIRS})

add_executable(fcitx-config-merge-translation main.c)
set_target_properties(fcitx-config-merge-translation PROPERTIES
                      OUTPUT_NAME fcitx-config-merge-translation)
target_link_libraries(fcitx-config-merge-translation Fcitx::Config ${GETTEXTPO_LIBRARIES})
add_executable(Fcitx::config-merge-translation ALIAS fcitx-config-merge-translation)
