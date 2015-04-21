#.rst:
# FindFcitx
# -------
#

if(CMAKE_VERSION VERSION_LESS 2.8.12)
    message(FATAL_ERROR "CMake 2.8.12 is required by FindFcitx.cmake")
endif()
if(CMAKE_MINIMUM_REQUIRED_VERSION VERSION_LESS 2.8.12)
    message(AUTHOR_WARNING "Your project should require at least CMake 2.8.12 to use FindFcitx.cmake")
endif()

if (NOT Fcitx_FIND_COMPONENTS)
    set(Fcitx_NOT_FOUND_MESSAGE "The Fcitx package requires at least one component")
    set(Fcitx_FOUND False)
    return()
endif()

set(_quiet_arg)
if (Fcitx_FIND_QUIETLY)
    set(_quiet_arg QUIET)
endif()
set(_exact_arg)
if (Fcitx_FIND_EXACT)
    set(_exact_arg EXACT)
endif()

include(FindPackageHandleStandardArgs)
include(FeatureSummary)

set(Fcitx_VERSION)
foreach(_module ${Fcitx_FIND_COMPONENTS})
    find_package(Fcitx${_module} ${Fcitx_FIND_VERSION}
        ${_exact_arg} ${_quiet_arg}
        CONFIG
    )
    find_package_handle_standard_args(Fcitx${_module} CONFIG_MODE)
    if (Fcitx_FIND_REQUIRED AND Fcitx_FIND_REQUIRED_${_module})
        # If the component was required, we tell FeatureSummary so that it
        # will be displayed in the correct list. We do not use the REQUIRED
        # argument of find_package() to allow all the missing frameworks
        # to be listed at once (fphsa will error out at the end of this file
        # anyway).
        set_package_properties(Fcitx${_module} PROPERTIES TYPE REQUIRED)
    endif()

    # Component-based find modules are expected to set
    # <module>_<component>_FOUND and <module>_<component>_VERSION variables,
    # but the find_package calls above will have set Fcitx<component>_*
    # variables.
    set(Fcitx_${_module}_FOUND ${Fcitx${_module}_FOUND})
    if(Fcitx${_module}_FOUND)
        set(Fcitx_${_module}_VERSION ${Fcitx${_module}_VERSION})

        # make Fcitx_VERSION the minimum found version
        if(NOT Fcitx_VERSION OR Fcitx_VERSION VERSION_GREATER Fcitx${_module}_VERSION)
            set(Fcitx_VERSION ${Fcitx${_module}_VERSION})
        endif()
    endif()
endforeach()

# Annoyingly, find_package_handle_standard_args requires you to provide
# REQUIRED_VARS even when using HANDLE_COMPONENTS, but all we actually
# care about is whether the required components were found. So we provide
# a dummy variable that is just set to something that will be printed
# on success.
set(_dummy_req_var "success")

find_package_handle_standard_args(Fcitx
    FOUND_VAR
        Fcitx_FOUND
    REQUIRED_VARS
        _dummy_req_var
    VERSION_VAR
        Fcitx_VERSION
    HANDLE_COMPONENTS
)

