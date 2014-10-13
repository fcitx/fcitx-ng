macro(FCITX5_GENERATE_CONFIG_SOURCE infile basename name prefix)
    get_filename_component(_in_file ${infile} ABSOLUTE)
    set(_target_c ${CMAKE_CURRENT_BINARY_DIR}/${basename}.c)
    set(_target_h ${CMAKE_CURRENT_BINARY_DIR}/${basename}.h)

    add_custom_command(OUTPUT ${_target_c}
        COMMAND Fcitx::configdesc-compiler -c -i "${basename}.h" -n "${name}" -p "${prefix}" -o ${_target_c} ${infile}
        DEPENDS ${_in_file} Fcitx::configdesc-compiler VERBATIM)

    add_custom_command(OUTPUT ${_target_h}
        COMMAND Fcitx::configdesc-compiler -n "${name}" -p "${prefix}" -o ${_target_h} ${infile}
        DEPENDS ${_in_file} Fcitx::configdesc-compiler VERBATIM)
endmacro()

macro(FCITX5_MERGE_CONFIG_TRANSLATION infile outfile)
    get_filename_component(abs_infile ${infile} ABSOLUTE)
    get_filename_component(infile_name ${infile} NAME)
    set(_outfile "${outfile}")
    if(NOT IS_ABSOLUTE "${outfile}")
        set(_outfile "${CMAKE_CURRENT_BINARY_DIR}/${outfile}")
    endif()

    add_custom_command(OUTPUT "${_outfile}"
        COMMAND Fcitx::config-merge-translation "${abs_infile}" "${PROJECT_SOURCE_DIR}/po" "${_outfile}"
        DEPENDS "${abs_infile}" Fcitx::config-merge-translation VERBATIM)

    add_custom_target("${infile_name}.target" ALL DEPENDS "${_outfile}")
endmacro()

macro(fcitx5_translate_add_po_file )
endmacro()

macro(fcitx5_translate_set_pot_target )
endmacro()
