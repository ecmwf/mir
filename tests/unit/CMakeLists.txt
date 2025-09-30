
# Setup cache directory
set(TEST_ECKIT_CACHE_DIR ${CMAKE_CURRENT_BINARY_DIR}/cache/)
file(REMOVE_RECURSE ${TEST_ECKIT_CACHE_DIR})

set(TESTS
#    MIR-333
#    MIR-356
    MIR-425
    MIR-536
    PGEN-412
    PGEN-492
    action_graph
    area
    atlas
    bounding_box
    earthkit-regrid
    formula
    gaussian_grid
    grib_basic_angle
    grib_encoding
    grib_input
    grid_box_method
    grids
    gridspec
    increments
    input
    interpolations
    interpolationspec
    iterator
    knn_weighting
    packing
    raw_memory
    spectral_order
    statistics
    style
    vector-space
    weight_matrix
    wind)

if(mir_HAVE_NETCDF)
    list(APPEND TESTS interpolation_netcdf)
endif()

foreach(_t ${TESTS})
    ecbuild_add_test(
        TARGET            mir_tests_unit_${_t}
        SOURCES           ${_t}.cc
        LIBS              mir
        ENVIRONMENT       ${_testEnvironment}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endforeach()

if(ENABLE_BUILD_TOOLS)
    ecbuild_configure_file(grib_fail_decode.sh.in grib_fail_decode.sh)

    foreach(_t
            MIR-637.001.grib2
            MIR-637.002.grib2)
        ecbuild_add_test(
            TARGET            mir_tests_unit_grib_fail_decode_${_t}
            COMMAND           ${CMAKE_CURRENT_BINARY_DIR}/grib_fail_decode.sh
            ARGS              --dryrun ${_t}
            ENVIRONMENT       ${_testEnvironment} MIR_GRIB_CHECK_IS_MESSAGE_VALID=1
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            TEST_PROPERTIES   WILL_FAIL TRUE
        )
    endforeach()

    foreach(_t
            gridName=F1280.area=55_65_0_145.grib1
            gridName=F1280.area=55_65_0_145.grib2)
        ecbuild_add_test(
            TARGET            mir_tests_unit_grib_decode_${_t}
            COMMAND           mir-get-data
            ARGS              ${_t}
            ENVIRONMENT       ${_testEnvironment}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    endforeach()
endif()
