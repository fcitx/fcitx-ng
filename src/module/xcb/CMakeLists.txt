include_directories(${PROJECT_BINARY_DIR}/src/lib/fcitx)
add_library(xcb MODULE xcbmodule.c)
target_link_libraries(xcb fcitx-core)
