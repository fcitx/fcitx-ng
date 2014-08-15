include_directories(${PROJECT_BINARY_DIR}/src/lib/fcitx)
add_library(testfrontend MODULE testfrontend.c)
add_library(testim STATIC testim.c)
