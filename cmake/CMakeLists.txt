install(FILES FcitxConfig.cmake FcitxMacro.cmake
  "${CMAKE_CURRENT_BINARY_DIR}/FcitxConfigVersion.cmake"
  DESTINATION "${FCITX_INSTALL_CMAKECONFIG_DIR}")
install(FILES
  fcitx-merge-config.sh
  fcitx-cmake-helper.sh
  fcitx-extract-gettext.sh
  fcitx-extract-confdesc.sh
  fcitx-extract-desktop.sh
  fcitx-extract-po.sh
  fcitx-extract-qt.sh
  fcitx-extract-kde.sh
  DESTINATION "${FCITX_INSTALL_CMAKECONFIG_DIR}"
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
  GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
install(FILES
  fcitx-parse-po.sh
  fcitx-write-po.sh
  DESTINATION "${FCITX_INSTALL_CMAKECONFIG_DIR}"
  PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
