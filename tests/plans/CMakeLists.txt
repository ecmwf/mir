find_program(DIFF_TOOL NAMES "diff" "cmp" "false")
find_program(MARS_SCRIPT NAMES "mars" "false")

ecbuild_configure_file(mir-test.sh.in mir-test.sh @ONLY)

file(GLOB_RECURSE test_files LIST_DIRECTORIES false *.test)
foreach(_t ${test_files})
    ecbuild_add_test(
        TARGET      ${_t}
        CONDITION   MARS_SCRIPT
        COMMAND     mir-test.sh
        ARGS        ${_t}
        ENVIRONMENT ${_testEnvironment})
endforeach()

if(EXISTS ${CMAKE_BINARY_DIR}/share/plugins/atlas-orca.yml)
    file(GLOB_RECURSE test_files LIST_DIRECTORIES false *.test.atlas-orca)
    foreach(_t ${test_files})
        ecbuild_add_test(
            TARGET      ${_t}
            CONDITION   MARS_SCRIPT
            COMMAND     mir-test.sh
            ARGS        ${_t}
            ENVIRONMENT ${_testEnvironment})
    endforeach()
endif()

set(_t ${CMAKE_CURRENT_SOURCE_DIR}/config)
ecbuild_add_test(
    TARGET      ${_t}
    CONDITION   MARS_SCRIPT
    COMMAND     mir-test.sh
    ARGS        ${_t}.x
    ENVIRONMENT ${_testEnvironment}
    MIR_CONFIG_AREA=${_t}/area.yaml
    MIR_CONFIG_PARAMETERS=${_t}/parameters.yaml)

