find_program(MARS_SCRIPT NAMES mars false)

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
           (_t MATCHES "[.]proj[.]" AND NOT atlas_HAVE_PROJ))
            set_tests_properties(${target} PROPERTIES WILL_FAIL TRUE)
        endif()
    endforeach()
endif()

if(NOT eccodes_HAVE_AEC)
    set_tests_properties(
	    mir_tests_assertions_MIR_488_001
	    mir_tests_assertions_MIR_488_002
	    mir_tests_assertions_MIR_513_003
	    mir_tests_assertions_MIR_513_004
	    mir_tests_assertions_MIR_513_packingType_grid_ccsds_001
	    mir_tests_assertions_MIR_513_packingType_grid_ccsds_002
	    mir_tests_assertions_MIR_513_packingType_grid_ccsds_004
	    mir_tests_assertions_MIR_513_packingType_grid_ccsds_005
	    mir_tests_assertions_MIR_513_packingType_grid_ccsds_007
	    mir_tests_assertions_MIR_513_packingType_grid_ieee_003
	    mir_tests_assertions_MIR_513_packingType_grid_second_order_003
	    mir_tests_assertions_MIR_513_packingType_grid_simple_003
	    mir_tests_assertions_MIR_513_packingType_grid_simple_011
	    mir_tests_assertions_MIR_513_packingType_grid_simple_016
	    mir_tests_assertions_MIR_513_packingType_grid_simple_019
	    mir_tests_assertions_MIR_513_packingType_spectral_complex_003
	    mir_tests_assertions_MIR_513_packingType_spectral_complex_017
	    mir_tests_assertions_MIR_513_packingType_spectral_complex_026
	    mir_tests_assertions_MIR_513_packingType_spectral_complex_032
	    mir_tests_assertions_MIR_513_packingType_spectral_complex_035
	    mir_tests_assertions_MIR_513_packingType_spectral_complex_037
	    mir_tests_assertions_MIR_513_packingType_spectral_complex_040
	    mir_tests_assertions_MIR_513_packingType_spectral_complex_041
	    mir_tests_assertions_MIR_513_packingType_spectral_simple_003
	    mir_tests_assertions_MIR_632_002
	    mir_tests_assertions_MIR_632_003
	    mir_tests_assertions_MIR_632_005
	    mir_tests_assertions_MIR_632_007
	    mir_tests_assertions_MIR_632_008
	    mir_tests_assertions_MIR_632_009
	    mir_tests_assertions_MIR_632_010
	    mir_tests_assertions_MIR_632_012
	    mir_tests_assertions_MIR_632_013
	    mir_tests_assertions_MIR_632_014
	    mir_tests_assertions_MIR_632_015
	    mir_tests_assertions_MIR_632_016
	    mir_tests_assertions_MIR_632_018
	    mir_tests_assertions_MIR_643_001
	    mir_tests_assertions_MIR_643_002
	    mir_tests_assertions_MIR_643_003
	    mir_tests_assertions_MIR_647_001
	    mir_tests_assertions_MIR_647_002
	    mir_tests_assertions_MIR_651_packingType_spectral_ieee_003
	    mir_tests_assertions_MIR_679_WP_to_ICON
	    mir_tests_assertions_MIR_683_001
	    mir_tests_assertions_MIR_683_002
	    mir_tests_assertions_MIR_684_005
	    mir_tests_assertions_MIR_684_006
	    mir_tests_assertions_MIR_684_007
	    mir_tests_assertions_MIR_692_003
	    mir_tests_assertions_bitmap_001
	    mir_tests_assertions_bitmap_002
	    mir_tests_assertions_bitmap_003
	    mir_tests_assertions_bitmap_004
	    mir_tests_assertions_bitmap_005
	    mir_tests_assertions_bitmap_006
	    mir_tests_assertions_grid_from_geopoints_format_ncols
	    mir_tests_assertions_grid_from_geopoints_format_polar_vector
	    mir_tests_assertions_grid_from_geopoints_format_xy_vector
	    mir_tests_assertions_grid_from_geopoints_format_xyv
	    mir_tests_assertions_grid_from_geopoints_standard
        PROPERTIES WILL_FAIL TRUE)
endif()

if(NOT atlas_HAVE_TESSELATION)
    set_tests_properties(
        mir_tests_assertions_MIR_647_001
        PROPERTIES WILL_FAIL TRUE)
endif()
