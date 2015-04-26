macro(fcitx5_generate_config_source infile basename name prefix)
    get_filename_component(_in_file ${infile} ABSOLUTE)
    set(_target_c ${CMAKE_CURRENT_BINARY_DIR}/${basename}.c)
    set(_target_h ${CMAKE_CURRENT_BINARY_DIR}/${basename}.h)

    add_custom_command(OUTPUT ${_target_h}
        COMMAND Fcitx5::configdesc-compiler -n "${name}" -p "${prefix}" -o ${_target_h} ${_in_file}
        DEPENDS ${_in_file} Fcitx5::configdesc-compiler VERBATIM)

    add_custom_command(OUTPUT ${_target_c}
        COMMAND Fcitx5::configdesc-compiler -c -i "${basename}.h" -n "${name}" -p "${prefix}" -o ${_target_c} ${_in_file}
        DEPENDS ${_in_file} ${_target_h} Fcitx5::configdesc-compiler ${_target_h} VERBATIM)
endmacro()

macro(fcitx5_generate_binary_file_header infile outfile varname)
    get_filename_component(_in_file ${infile} ABSOLUTE)
    add_custom_command(OUTPUT ${outfile}
                       COMMAND Fcitx5::text2cstring varname ${_in_file} > ${outfile}
                       DEPENDS ${_in_file})
endmacro()

macro(fcitx5_merge_config_translation infile outfile)
    get_filename_component(abs_infile ${infile} ABSOLUTE)
    get_filename_component(infile_name ${infile} NAME)
    set(_outfile "${outfile}")
    if(NOT IS_ABSOLUTE "${outfile}")
        set(_outfile "${CMAKE_CURRENT_BINARY_DIR}/${outfile}")
    endif()

    add_custom_command(OUTPUT "${_outfile}"
        COMMAND Fcitx5::config-merge-translation "${abs_infile}" "${PROJECT_SOURCE_DIR}/po" "${_outfile}"
        DEPENDS "${abs_infile}" Fcitx5::config-merge-translation VERBATIM)

    add_custom_target("${infile_name}.target" ALL DEPENDS "${_outfile}")
endmacro()

macro(fcitx5_generate_addon_function infile)
    get_filename_component(abs_infile ${infile} ABSOLUTE)
    get_filename_component(infile_name ${infile} NAME_WE)

    add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${infile_name}-internal.h"
        COMMAND Fcitx5::addon-function-compiler -i "${abs_infile}" "${CMAKE_CURRENT_BINARY_DIR}/${infile_name}-internal.h"
        DEPENDS "${abs_infile}" Fcitx5::addon-function-compiler VERBATIM)

    add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${infile_name}.h"
        COMMAND Fcitx5::addon-function-compiler "${abs_infile}" "${CMAKE_CURRENT_BINARY_DIR}/${infile_name}.h"
        DEPENDS "${abs_infile}" Fcitx5::addon-function-compiler VERBATIM)
    add_custom_target("${infile_name}.h.target" ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${infile_name}.h" "${CMAKE_CURRENT_BINARY_DIR}/${infile_name}-internal.h")

    add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${infile_name}.c"
        COMMAND Fcitx5::addon-function-compiler -c "${abs_infile}" "${CMAKE_CURRENT_BINARY_DIR}/${infile_name}.c"
        DEPENDS "${abs_infile}" "${CMAKE_CURRENT_BINARY_DIR}/${infile_name}-internal.h" Fcitx5::addon-function-compiler VERBATIM)
endmacro()

macro(fcitx5_translate_add_po_file )
endmacro()

macro(fcitx5_translate_set_pot_target )
endmacro()

macro(fcitx5_install_addon_config name)
fcitx5_merge_config_translation("${name}.conf.in" "${name}.conf")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${name}.conf" DESTINATION "${FCITX_INSTALL_PKGDATADIR}/addon")
endmacro()
