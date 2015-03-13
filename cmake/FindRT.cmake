# - find where clock_gettime and friends are located.
# RT_FOUND - system has dynamic linking interface available
# RT_LIBRARY - libraries needed to use dlopen

include(CheckFunctionExists)
include(CheckLibraryExists)

find_library(RT_LIBRARY NAMES rt)
if(RT_LIBRARY)
  check_library_exists(rt clock_gettime "time.h" RT_CLOCK_GETTIME_FOUND)
else(RT_LIBRARY)
  check_function_exists(clock_gettime RT_CLOCK_GETTIME_FOUND)
  # If dlopen can be found without linking in dl then dlopen is part
  # of libc, so don't need to link extra libs.
  set(RT_LIBRARY "")
endif(RT_LIBRARY)

mark_as_advanced(RT_CLOCK_GETTIME_FOUND RT_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(RT
    FOUND_VAR
        RT_FOUND
    REQUIRED_VARS
        RT_CLOCK_GETTIME_FOUND
)

if(RT_FOUND AND NOT TARGET RT::RT)
    if (RT_LIBRARY)
        add_library(RT::RT UNKNOWN IMPORTED)
        set_target_properties(RT::RT PROPERTIES
            IMPORTED_LOCATION "${RT_LIBRARY}")
    else()
        add_library(RT::RT INTERFACE IMPORTED )
    endif()
    set_target_properties(RT::RT PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${RT_INCLUDE_DIR}"
    )
endif()
