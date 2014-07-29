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
