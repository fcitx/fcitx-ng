include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PROJECT_BINARY_DIR}/src/lib/fcitx)

fcitx5_generate_addon_function(testim.fxaddon)
add_library(testfrontend MODULE testfrontend.c)
add_library(testim STATIC testimmodule.c
            ${CMAKE_CURRENT_BINARY_DIR}/testim.c ${CMAKE_CURRENT_BINARY_DIR}/testim-internal.h)
target_link_libraries(testfrontend fcitx-core)
target_link_libraries(testim fcitx-core)
