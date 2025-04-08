ecbuild_configure_file(mir-test.sh.in mir-test.sh @ONLY)

file(GLOB_RECURSE test_files LIST_DIRECTORIES false *.test *.fail)

foreach(_t ${test_files})
    target_from_path("${_t}" target)
    ecbuild_add_test(
        TARGET      ${target}
        COMMAND     mir-test.sh
        ARGS        ${_t}
        ENVIRONMENT ${_testEnvironment})
    if((_t MATCHES "[.]fail$") OR
       (_t MATCHES "[.]lsm[.]" AND NOT HAVE_MIR_DOWNLOAD_MASKS))
        set_tests_properties(${_t} PROPERTIES WILL_FAIL TRUE)
    endif()
endforeach()
