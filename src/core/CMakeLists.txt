set(FCITX_SOURCES
  main.c
  errorhandler.c
  )

add_executable(fcitx ${FCITX_SOURCES})
install(TARGETS fcitx RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
target_link_libraries(fcitx fcitx-core LibExecinfo::LibExecinfo)
