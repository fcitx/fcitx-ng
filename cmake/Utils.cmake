find_package(PkgConfig)

macro(pkg_check_variable _var _pkglist _varname)
  set(_pkgconfig_invoke_result)

  execute_process(
    COMMAND ${PKG_CONFIG_EXECUTABLE} ${_pkglist} "--variable=${_varname}"
    OUTPUT_VARIABLE _pkgconfig_invoke_result
    RESULT_VARIABLE _pkgconfig_failed)

  if (_pkgconfig_failed)
    set(${_var} "" CACHE INTERNAL "")
  else()
    string(REGEX REPLACE "[\r\n]"                  " " _pkgconfig_invoke_result "${_pkgconfig_invoke_result}")
    string(REGEX REPLACE " +$"                     ""  _pkgconfig_invoke_result "${_pkgconfig_invoke_result}")
    set(${_var} "${_pkgconfig_invoke_result}" CACHE INTERNAL "")
  endif()
endmacro()
