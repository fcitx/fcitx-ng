cmake_minimum_required(VERSION 3.0)
set(FCITX_VERSION 4.99.0)
project(fcitx VERSION ${FCITX_VERSION})

find_package(ECM 0.0.11 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
set(CMAKE_AUTOMOC_RELAXED_MODE TRUE)

include(FeatureSummary)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMOptionalAddSubdirectory)
include(Utils)

include(Fcitx5Macros)

option(ENABLE_TEST "Build Test" On)
option(BUILD_COVERAGE "Build the project with gcov support" OFF)

if(NOT NO_SNOOPER_APPS)
  set(NO_SNOOPER_APPS ".*chrome.*,.*chromium.*,firefox.*,Do.*"
    CACHE STRING "Disable Key Snooper for following app")
endif()

if(NOT NO_PREEDIT_APPS)
  set(NO_PREEDIT_APPS "gvim"
    CACHE STRING "Disable Preedit for the following apps")
endif()

set_property(GLOBAL PROPERTY "__FCITX_INTERNAL_BUILD" On)

include(CheckCCompilerFlag)
include(CheckFunctionExists)

check_c_compiler_flag("-std=c99" C99_SUPPORTED)
if (NOT C99_SUPPORTED)
    message(FATAL_ERROR "C99 is required to compile Fcitx.")
endif()
check_function_exists(open_memstream HAVE_OPEN_MEMSTREAM)
check_function_exists(strtok_r HAVE_STRTOK_R)

include(GNUInstallDirs)

# directory needed by bsd
if(NOT CMAKE_INSTALL_LIBDATADIR)
  set(CMAKE_INSTALL_LIBDATADIR "${CMAKE_INSTALL_LIBDIR}" CACHE PATH "libdata root (LIBDIR)")
  set(CMAKE_INSTALL_LIBDATADIR "${CMAKE_INSTALL_LIBDIR}")
  mark_as_advanced(CMAKE_INSTALL_LIBDATADIR)
endif()

set(FCITX_INSTALL_LIBDIR ${CMAKE_INSTALL_FULL_LIBDIR})
set(FCITX_INSTALL_LIBDATADIR ${CMAKE_INSTALL_FULL_LIBDATADIR})
set(FCITX_INSTALL_DATADIR ${CMAKE_INSTALL_FULL_DATADIR})
set(FCITX_INSTALL_PKGDATADIR "${CMAKE_INSTALL_FULL_DATADIR}/fcitx5")
set(FCITX_INSTALL_BINDIR ${CMAKE_INSTALL_FULL_BINDIR})
set(FCITX_INSTALL_LOCALEDIR ${CMAKE_INSTALL_FULL_LOCALEDIR})
set(FCITX_INSTALL_ADDONDIR "${CMAKE_INSTALL_FULL_LIBDIR}/fcitx5")
set(FCITX_INSTALL_CMAKECONFIG_DIR "${CMAKE_INSTALL_FULL_LIBDIR}/cmake/Fcitx5")
set(FCITX_INSTALL_MODULE_HEADER_DIR "${CMAKE_INSTALL_FULL_INCLUDEDIR}/Fcitx5/FcitxCore/fcitx/module")

# TODO mac/win
set(FCITX_LIBRARY_SUFFIX ".so")

# process rpath
list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${FCITX_INSTALL_LIBDIR}" _isSystemPlatformLibDir)
list(FIND CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES "${FCITX_INSTALL_LIBDIR}" _isSystemCxxLibDir)
if("${_isSystemPlatformLibDir}" STREQUAL "-1" AND "${_isSystemCxxLibDir}" STREQUAL "-1")
    set(CMAKE_SKIP_BUILD_RPATH  FALSE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_LIBDIR}")
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif("${_isSystemPlatformLibDir}" STREQUAL "-1" AND "${_isSystemCxxLibDir}" STREQUAL "-1")

find_package(PkgConfig REQUIRED)
find_package(DL REQUIRED)
find_package(LibFFI REQUIRED)
find_package(LibUUID REQUIRED)
find_package(GettextPo REQUIRED)
find_package(RT)
find_package(LibExecinfo)
find_package(XKBCommon 0.5.0 REQUIRED COMPONENTS XKBCommon X11)
find_package(LibXml2 REQUIRED)

########### optional dependency ################
find_package(DBus)
find_package(XCB COMPONENTS XCB AUX XKB)
find_package(XCBImdkit)
find_package(Wayland COMPONENTS Client)

# we want some useful function like asprintf
add_definitions(-D_GNU_SOURCE)
set(CMAKE_C_FLAGS "-fPIC -std=c99 -Wall -Wextra -fvisibility=hidden ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "-fPIC -Wall -Wextra -fvisibility=hidden ${CMAKE_CXX_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--as-needed -Wl,--no-undefined ${CMAKE_SHARED_LINKER_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS "-Wl,--as-needed -Wl,--no-undefined ${CMAKE_MODULE_LINKER_FLAGS}")

if(BUILD_COVERAGE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lgcov")
endif()


configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

ecm_setup_version(PROJECT
                  VARIABLE_PREFIX "FCITX"
                  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/fcitx5_version.h"
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/Fcitx5ConfigVersion.cmake")

include(CMakePackageConfigHelpers)

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/Fcitx5Config.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/Fcitx5Config.cmake"
                              INSTALL_DESTINATION  ${FCITX_INSTALL_CMAKECONFIG_DIR}
                              )


install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/Fcitx5ConfigVersion.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/Fcitx5Config.cmake"
               "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Fcitx5Macros.cmake"
        DESTINATION "${FCITX_INSTALL_CMAKECONFIG_DIR}"
        COMPONENT Devel )


install(FILES ${CMAKE_CURRENT_BINARY_DIR}/fcitx5_version.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} COMPONENT Devel )

add_subdirectory(src)

if (ENABLE_TEST)
    enable_testing()
    add_subdirectory(test)

    if (BUILD_COVERAGE)
        add_custom_target(coverage
                        COMMAND "${CMAKE_CTEST_COMMAND}"
                        COMMAND lcov --capture --directory ./ --output-file coverage.info
                        COMMAND genhtml coverage.info --output-directory "coverage_pages"
                        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    endif()
endif ()

add_subdirectory(tools)

add_subdirectory(data)

add_subdirectory(po)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
