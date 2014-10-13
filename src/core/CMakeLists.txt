set(FCITX_SOURCES
  main.c
  errorhandler.c
  )

add_executable(fcitx ${FCITX_SOURCES})
install(TARGETS fcitx RUNTIME DESTINATION "${FCITX_INSTALL_BINDIR}")
target_link_libraries(fcitx fcitx-core ${LIBEXECINFO_LIBRARIES})
