
set(FCITX_UTILS_TEST
    testarray
    testhandlertable
    testkey
    testobjpool
    testsort
    teststring
    testdict
    testmemorypool
    testfs
    testutf8
    testlibrary
    teststandardpath
    testmainloop
    testlist
    testi18nstring
    testcolor)

foreach(TESTCASE ${FCITX_UTILS_TEST})
    add_executable(${TESTCASE} ${TESTCASE}.c)
    target_link_libraries(${TESTCASE} Fcitx::Utils)
    add_test(NAME ${TESTCASE}
             COMMAND ${TESTCASE})
endforeach()

add_executable(testcomposetable testcomposetable.c)
target_link_libraries(testcomposetable Fcitx::ComposeTable)

add_executable(testconfiguration testconfiguration.c)
target_link_libraries(testconfiguration Fcitx::Config)
add_test(NAME testconfiguration COMMAND testconfiguration)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

# test desc compiler
set(TEST_DESC_COMPILE_SOURCE
  ${CMAKE_CURRENT_BINARY_DIR}/test-config.c
  ${CMAKE_CURRENT_BINARY_DIR}/test-config.h
  testdesccompile.c
  )

add_executable(testdesccompile ${TEST_DESC_COMPILE_SOURCE})
target_link_libraries(testdesccompile Fcitx::Config)

fcitx5_generate_config_source(${CMAKE_CURRENT_SOURCE_DIR}/test.desc test-config TestConfig Fcitx)
add_test(NAME testdesccompile COMMAND testdesccompile)

# test addon
add_executable(testaddon testaddon.c)
target_link_libraries(testaddon Fcitx::Core testim)
target_include_directories(testaddon PRIVATE ${PROJECT_BINARY_DIR}/src/lib/fcitx)
add_test(NAME testaddon COMMAND testaddon ${CMAKE_CURRENT_SOURCE_DIR}/addon ${CMAKE_CURRENT_BINARY_DIR}/addon)

add_subdirectory(addon)
