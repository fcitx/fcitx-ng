# - find where clock_gettime and friends are located.
# RT_FOUND - system has dynamic linking interface available
# RT_LIBRARIES - libraries needed to use dlopen

include(CheckFunctionExists)
include(CheckLibraryExists)

find_library(RT_LIBRARIES NAMES rt)
if(RT_LIBRARIES)
  check_library_exists(rt clock_gettime "time.h" RT_FOUND)
else(RT_LIBRARIES)
  check_function_exists(dlopen RT_FOUND)
  # If dlopen can be found without linking in dl then dlopen is part
  # of libc, so don't need to link extra libs.
  set(RT_LIBRARIES "")
endif(RT_LIBRARIES)
