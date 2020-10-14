find_program(DIFF_TOOL NAMES "diff" "cmp" "false")
find_program(MARS_SCRIPT NAMES "mars" "false")

ecbuild_configure_file(mir-test.sh.in mir-test.sh @ONLY)

file(GLOB_RECURSE test_files LIST_DIRECTORIES false *.test)

foreach(_t ${test_files})
    ecbuild_add_test(
        TARGET      ${_t}
        CONDITION   MARS_SCRIPT AND HAVE_TEST_ASSERTIONS
        COMMAND     mir-test.sh
        ARGS        ${_t}
        ENVIRONMENT ${_testEnvironment})
endforeach()

