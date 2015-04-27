fcitx5_generate_binary_file_header(testim.conf testim_conf.h testim_conf)
fcitx5_generate_addon_function(testim.fxaddon)
add_library(testfrontend MODULE testfrontend.c)
add_library(testim STATIC testimmodule.c ${CMAKE_CURRENT_BINARY_DIR}/testim_conf.h
            ${CMAKE_CURRENT_BINARY_DIR}/testim.c ${CMAKE_CURRENT_BINARY_DIR}/testim-internal.h)
target_include_directories(testim PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(testfrontend Fcitx5::Core)
target_link_libraries(testim Fcitx5::Core)
