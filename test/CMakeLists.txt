
set(FCITX_UTILS_TEST
    testarray
    testhandlertable
    testkey
    testobjpool
    testsort
    teststring
    testdict
    testmemorypool
    testutf8)

foreach(TESTCASE ${FCITX_UTILS_TEST})
    add_executable(${TESTCASE} ${TESTCASE}.c)
    target_link_libraries(${TESTCASE} fcitx-utils)
    add_test(NAME ${TESTCASE}
             COMMAND ${TESTCASE})
endforeach()
