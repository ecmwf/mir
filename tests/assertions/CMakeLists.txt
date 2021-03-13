find_program(MARS_SCRIPT NAMES mars false)

ecbuild_configure_file(mir-test.sh.in mir-test.sh @ONLY)

if(MARS_SCRIPT)
    file(GLOB_RECURSE test_files LIST_DIRECTORIES false *.test)

    foreach(_t ${test_files})
        ecbuild_add_test(
            TARGET      ${_t}
            COMMAND     mir-test.sh
            ARGS        ${_t}
            ENVIRONMENT ${_testEnvironment})
        if((_t MATCHES "[.]fail[.]") OR
           (_t MATCHES "[.]lsm[.]" AND NOT HAVE_MIR_DOWNLOAD_MASKS) OR
           (NOT mir_HAVE_ATLAS AND NOT (_t MATCHES "[.]core[.]")))
            set_tests_properties(${_t} PROPERTIES WILL_FAIL TRUE)
        endif()
    endforeach()
endif()

if(NOT eccodes_HAVE_AEC)
    foreach(_r
        MIR-513.packingType=grid_ccsds.001.test
        MIR-513.packingType=grid_ccsds.002.test
        MIR-513.packingType=grid_ccsds.004.test
        MIR-513.packingType=grid_ccsds.005.test
        MIR-513.packingType=grid_ieee.003.test
        MIR-513.packingType=grid_second_order.003.test
        MIR-513.packingType=grid_simple.003.test)
        get_filename_component(_t "${_r}" ABSOLUTE)
        set_tests_properties(${_t} PROPERTIES WILL_FAIL TRUE)
    endforeach()
endif()

