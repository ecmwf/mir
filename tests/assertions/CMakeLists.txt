
# set(MIR_TOOL     "$<TARGET_FILE:mir-tool>")
# set(MIR_COMPARE  "$<TARGET_FILE:mir-compare>")
# set(GRIB_GET     "$<TARGET_FILE:grib_get>")
# set(GRIB_COMPARE "$<TARGET_FILE:grib_compare>")

get_target_property(MIR_TOOL     "mir-tool"     LOCATION)
get_target_property(MIR_COMPARE  "mir-compare"  LOCATION)
get_target_property(GRIB_GET     "grib_get"     LOCATION)
get_target_property(GRIB_COMPARE "grib_compare" LOCATION)

find_program(MARS_SCRIPT NAMES mars false)

configure_file(mir-test.sh.in mir-test.sh @ONLY)

file(GLOB_RECURSE test_files LIST_DIRECTORIES false *.test *.fail)

if(MARS_SCRIPT)
    foreach(_t ${test_files})
        ecbuild_add_test(
            TARGET      ${_t}
            COMMAND     mir-test.sh
            ARGS        ${_t}
            ENVIRONMENT ${_testEnvironment})
        if((_t MATCHES "[.]fail$") OR
           (_t MATCHES "[.]lsm[.]" AND NOT HAVE_MIR_DOWNLOAD_MASKS) OR
           (_t MATCHES "[.]atlas[.]" AND NOT HAVE_ATLAS))
            set_tests_properties(${_t} PROPERTIES WILL_FAIL TRUE)
        endif()
    endforeach()
endif()
