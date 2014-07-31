
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
    target_link_libraries(${TESTCASE} fcitx-utils)
    add_test(NAME ${TESTCASE}
             COMMAND ${TESTCASE})
endforeach()

add_executable(testcomposetable testcomposetable.c)
target_link_libraries(testcomposetable fcitx-composetable)

add_executable(testconfiguration testconfiguration.c)
target_link_libraries(testconfiguration fcitx-config)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(TEST_DESC_COMPILE_SOURCE
  ${CMAKE_CURRENT_BINARY_DIR}/test-config.c
  ${CMAKE_CURRENT_BINARY_DIR}/test-config.h
  testdesccompile.c
  )

add_executable(testdesccompile ${TEST_DESC_COMPILE_SOURCE})
target_link_libraries(testdesccompile fcitx-config)

fcitx5_generate_config_source(${CMAKE_CURRENT_SOURCE_DIR}/test.desc test-config TestConfig Fcitx)
