add_library(testfrontend MODULE testfrontend.c)
target_include_directories(testfrontend PRIVATE ${PROJECT_BINARY_DIR}/src/lib/fcitx)

