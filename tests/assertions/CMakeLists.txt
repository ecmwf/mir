find_program(MARS_SCRIPT NAMES mars)

ecbuild_configure_file(mir-test.sh.in mir-test.sh @ONLY)

file(CREATE_LINK
    "${CMAKE_BINARY_DIR}/etc/mir/grib-output-ii.yaml"
    "${CMAKE_CURRENT_BINARY_DIR}/grib-output-ii.yaml"
    COPY_ON_ERROR SYMBOLIC)

file(CREATE_LINK
    "${CMAKE_BINARY_DIR}/etc/mir/grib-output-iii.yaml"
    "${CMAKE_CURRENT_BINARY_DIR}/grib-output-iii.yaml"
    COPY_ON_ERROR SYMBOLIC)

if(MARS_SCRIPT)
    file(GLOB_RECURSE test_files LIST_DIRECTORIES false *.test)

    foreach(_t ${test_files})
        target_from_path("${_t}" target)
        ecbuild_add_test(
            TARGET      ${target}
            COMMAND     mir-test.sh
            ARGS        ${_t}
            ENVIRONMENT ${_testEnvironment})
        if((_t MATCHES "[.]fail[.]") OR
           (_t MATCHES "[.]lsm[.]" AND NOT HAVE_MIR_DOWNLOAD_MASKS) OR
           (NOT mir_HAVE_ATLAS AND NOT (_t MATCHES "[.]core[.]")) OR
           (NOT atlas_HAVE_PROJ AND (_t MATCHES "[.]proj[.]")))
            set_tests_properties(${target} PROPERTIES WILL_FAIL TRUE)
        endif()
    endforeach()
endif()

if(NOT eccodes_HAVE_AEC)
    foreach(_r
        MIR-513.packingType=grid_ccsds.001.core.test
        MIR-513.packingType=grid_ccsds.002.core.test
        MIR-513.packingType=grid_ccsds.004.core.test
        MIR-513.packingType=grid_ccsds.005.core.test
        MIR-513.packingType=grid_ieee.003.core.test
        MIR-513.packingType=grid_second_order.003.core.test
        MIR-513.packingType=grid_simple.003.core.test)
        get_filename_component(_t "${_r}" ABSOLUTE)
        target_from_path("${_t}" target)
        set_tests_properties(${target} PROPERTIES WILL_FAIL TRUE)
    endforeach()
endif()

if(NOT atlas_HAVE_TESSELATION)
    foreach(_r
            MIR-647.001.test)
        get_filename_component(_t "${_r}" ABSOLUTE)
        target_from_path("${_t}" target)
        if(${target})
            set_tests_properties(${target} PROPERTIES WILL_FAIL TRUE)
        endif()
    endforeach()
endif()
