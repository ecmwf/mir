find_program(DIFF_TOOL NAMES "diff" "cmp" "false")
find_program(MARS_SCRIPT NAMES "mars" "false")

ecbuild_configure_file(mir-test.sh.in mir-test.sh @ONLY)

if(MARS_SCRIPT)
    file(GLOB_RECURSE test_files LIST_DIRECTORIES false *.test)

    foreach(_t ${test_files})
        target_from_path("${_t}" target)
        ecbuild_add_test(
            TARGET      ${target}
            CONDITION   MARS_SCRIPT
            COMMAND     mir-test.sh
            ARGS        ${_t}
            ENVIRONMENT ${_testEnvironment})
        if((_t MATCHES "[.]fail[.]") OR
           (_t MATCHES "[.]lsm[.]" AND NOT HAVE_MIR_DOWNLOAD_MASKS) OR
           (_t MATCHES "[.]proj[.]" AND NOT atlas_HAVE_PROJ))
            set_tests_properties(${target} PROPERTIES WILL_FAIL TRUE)
        endif()
    endforeach()
endif()

set(_t ${CMAKE_CURRENT_SOURCE_DIR}/config)
target_from_path("${_t}" target)
ecbuild_add_test(
    TARGET      ${target}
    CONDITION   MARS_SCRIPT
    COMMAND     mir-test.sh
    ARGS        ${_t}.x
    ENVIRONMENT ${_testEnvironment}
    MIR_CONFIG_AREA=${_t}/area.yaml
    MIR_CONFIG_PARAMETERS=${_t}/parameters.yaml)
