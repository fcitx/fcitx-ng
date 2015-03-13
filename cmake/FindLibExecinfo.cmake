find_path(LIBEXECINFO_INCLUDE_DIR execinfo.h)

if(LIBEXECINFO_INCLUDE_DIR)
  include(CheckFunctionExists)
  check_function_exists(backtrace LIBEXECINFO_LIBC_HAS_LIBEXECINFO_BACKTRACE)

  if (LIBEXECINFO_LIBC_HAS_LIBEXECINFO_BACKTRACE)
    set(LIBEXECINFO_LIBRARY "")
  else (LIBEXECINFO_LIBC_HAS_LIBEXECINFO_BACKTRACE)
    find_library(LIBEXECINFO_LIBRARY NAMES execinfo libexecinfo )
  endif (LIBEXECINFO_LIBC_HAS_LIBEXECINFO_BACKTRACE)

endif(LIBEXECINFO_INCLUDE_DIR)

mark_as_advanced(LIBEXECINFO_INCLUDE_DIR  LIBEXECINFO_LIBRARY  LIBEXECINFO_LIBC_HAS_LIBEXECINFO_BACKTRACE)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibExecinfo
    FOUND_VAR
        LIBEXECINFO_FOUND
    REQUIRED_VARS
        LIBEXECINFO_INCLUDE_DIR
)

if(LIBEXECINFO_FOUND AND NOT TARGET LibExecinfo::LibExecinfo)
    if (LIBEXECINFO_LIBRARY)
        add_library(LibExecinfo::LibExecinfo UNKNOWN IMPORTED)
        set_target_properties(LibExecinfo::LibExecinfo PROPERTIES
            IMPORTED_LOCATION "${LIBEXECINFO_LIBRARY}")
    else()
        add_library(LibExecinfo::LibExecinfo INTERFACE IMPORTED)
    endif()
    set_target_properties(LibExecinfo::LibExecinfo PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${LIBEXECINFO_INCLUDE_DIR}"
    )
endif()
