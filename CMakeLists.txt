cmake_minimum_required(VERSION 2.8.12)
project(fcitx)

set(FCITX_VERSION "4.99")

find_package(ECM 0.0.11 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
set(CMAKE_AUTOMOC_RELAXED_MODE TRUE)

include(FcitxMacros)

option(ENABLE_TEST "Build Test" On)
option(ENABLE_COVERAGE "Enable coverage" Off)
option(ENABLE_BACKTRACE "Build with backtrace support" On)
option(ENABLE_LIBXML2 "Build with libxml" On)

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

macro(MAKE_INSTALL_PATH_ABSOLUTE out in)
   if (IS_ABSOLUTE "${in}")    # IS_ABSOLUTE is new since cmake 2.4.8
      set(${out} "${in}")
   else()
      set(${out} "${CMAKE_INSTALL_PREFIX}/${in}")
   endif()
endmacro()

make_install_path_absolute(FCITX_INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR})
make_install_path_absolute(FCITX_INSTALL_LIBDATADIR ${CMAKE_INSTALL_LIBDATADIR})
make_install_path_absolute(FCITX_INSTALL_DATADIR ${CMAKE_INSTALL_DATADIR})
make_install_path_absolute(FCITX_INSTALL_PKGDATADIR "${CMAKE_INSTALL_DATADIR}/fcitx")
make_install_path_absolute(FCITX_INSTALL_BINDIR ${CMAKE_INSTALL_BINDIR})
make_install_path_absolute(FCITX_INSTALL_LOCALEDIR ${CMAKE_INSTALL_LOCALEDIR})
make_install_path_absolute(FCITX_INSTALL_ADDONDIR "${CMAKE_INSTALL_LIBDIR}/fcitx")

# TODO mac/win
set(FCITX_LIBRARY_SUFFIX ".so")

# process rpath
list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${FCITX_INSTALL_LIBDIR}" _isSystemPlatformLibDir)
list(FIND CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES "${FCITX_INSTALL_LIBDIR}" _isSystemCxxLibDir)
if("${_isSystemPlatformLibDir}" STREQUAL "-1" AND "${_isSystemCxxLibDir}" STREQUAL "-1")
    set(CMAKE_SKIP_BUILD_RPATH  FALSE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
    set(CMAKE_INSTALL_RPATH "${FCITX_INSTALL_LIBDIR}")
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif("${_isSystemPlatformLibDir}" STREQUAL "-1" AND "${_isSystemCxxLibDir}" STREQUAL "-1")

find_package(DL REQUIRED)
find_package(LibFFI REQUIRED)
find_package(LibUUID REQUIRED)
find_package(GettextPo REQUIRED)
find_package(RT)

if (ENABLE_BACKTRACE)
find_package(LibExecinfo REQUIRED)
endif ()

# Dependences for modules
# xcb
find_package(XCB REQUIRED COMPONENTS XCB XKB AUX)
find_package(XKeyboardConfig REQUIRED)
# xim
find_package(XCBImdkit REQUIRED)
# dbus
find_package(DBus REQUIRED)
# libxml
if (ENABLE_LIBXML2)
find_package(LibXml2 REQUIRED)
find_package(IsoCodes REQUIRED)
endif (ENABLE_LIBXML2)
# xkbcommon
find_package(XKBCommon REQUIRED)

if (RT_FOUND)
    set(HAVE_CLOCK_GETTIME 1)
else()
    set(HAVE_CLOCK_GETTIME 0)
endif()

# we want some useful function like asprintf
add_definitions(-D_GNU_SOURCE)
set(CMAKE_C_FLAGS "-fPIC -std=c99 -Wall -Wextra -fvisibility=hidden ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "-fPIC -Wall -Wextra -fvisibility=hidden ${CMAKE_CXX_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--as-needed -Wl,--no-undefined ${CMAKE_SHARED_LINKER_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS "-Wl,--as-needed -Wl,--no-undefined ${CMAKE_MODULE_LINKER_FLAGS}")

if (ENABLE_COVERAGE)
    add_definitions(-fprofile-arcs -ftest-coverage)
    set(CMAKE_EXE_LINKER_FLAGS "-fprofile-arcs -ftest-coverage ${CMAKE_EXE_LINKER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS "-fprofile-arcs -ftest-coverage ${CMAKE_SHARED_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS "-fprofile-arcs -ftest-coverage ${CMAKE_MODULE_LINKER_FLAGS}")
    add_custom_target(coverage
                      COMMAND "${CMAKE_CTEST_COMMAND}"
                      COMMAND lcov --capture --directory ./ --output-file coverage.info
                      COMMAND genhtml coverage.info --output-directory "coverage_pages"
                      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif()

include_directories(src/lib)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(src)

if (ENABLE_TEST)
    enable_testing()
    add_subdirectory(test)
endif ()

add_subdirectory(tools)

add_subdirectory(data)

add_subdirectory(po)
