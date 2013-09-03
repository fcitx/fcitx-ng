
add_executable(testsort testsort.c)
target_link_libraries(testsort fcitx-utils)

add_executable(testhotkey testhotkey.c)
target_link_libraries(testhotkey fcitx-utils)

add_executable(testutf8 testutf8.c)
target_link_libraries(testutf8 fcitx-utils)

add_test(NAME testutf8
         COMMAND testutf8)

add_test(NAME testhotkey
         COMMAND testhotkey)

add_test(NAME testsort
         COMMAND testsort)
