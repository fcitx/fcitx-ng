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

add_executable(testconfiguration testconfiguration.c)
target_link_libraries(testconfiguration Fcitx::Config)
add_test(NAME testconfiguration COMMAND testconfiguration)

# test desc compiler
set(TEST_DESC_COMPILE_SOURCE
  ${CMAKE_CURRENT_BINARY_DIR}/test-config.c
  ${CMAKE_CURRENT_BINARY_DIR}/test-config.h
  testdesccompile.c
  )

add_executable(testdesccompile ${TEST_DESC_COMPILE_SOURCE})
target_include_directories(testdesccompile PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(testdesccompile Fcitx::Config)

fcitx5_generate_config_source(${CMAKE_CURRENT_SOURCE_DIR}/test.desc test-config TestConfig Fcitx)
add_test(NAME testdesccompile COMMAND testdesccompile)

# test addon
add_executable(testaddon testaddon.c)
add_dependencies(testaddon testim.h.target)
target_include_directories(testaddon PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(testaddon Fcitx::Core testim)
add_test(NAME testaddon COMMAND testaddon ${CMAKE_CURRENT_SOURCE_DIR}/addon ${CMAKE_CURRENT_BINARY_DIR}/addon)

add_subdirectory(addon)

set(FCITX_UTILS_TEST
    testinputcontext
    testinputmethod
    testtext)

foreach(TESTCASE ${FCITX_CORE_TEST})
    add_executable(${TESTCASE} ${TESTCASE}.c)
    target_link_libraries(${TESTCASE} Fcitx::Core)
    add_test(NAME ${TESTCASE}
             COMMAND ${TESTCASE})
endforeach()
