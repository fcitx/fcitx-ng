set(FCITX_SOURCES
  main.c
  errorhandler.c
  )

add_executable(fcitx5 ${FCITX_SOURCES})
install(TARGETS fcitx5 RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
target_link_libraries(fcitx5 Fcitx5::Core LibExecinfo::LibExecinfo)
