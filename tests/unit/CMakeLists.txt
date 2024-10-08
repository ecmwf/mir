
# Setup cache directory
set(TEST_ECKIT_CACHE_DIR ${CMAKE_CURRENT_BINARY_DIR}/cache/)
file(REMOVE_RECURSE ${TEST_ECKIT_CACHE_DIR})

foreach(_t
#    MIR-333
#    MIR-356
    MIR-425
    MIR-536
    PGEN-412
    PGEN-492
    action_graph
    area
    bounding_box
    formula
    gaussian_grid
    grib_basic_angle
    grib_encoding
    grib_input
    grid_box_method
    increments
    input_MultiDimensionalInput
    interpolations
    iterator
    knn_weighting
    packing
    raw_memory
    spectral_order
    statistics
    style
    vector-space
    wind)
    ecbuild_add_test(
        TARGET            mir_tests_unit_${_t}
        SOURCES           ${_t}.cc
        LIBS              mir
        ENVIRONMENT       ${_testEnvironment}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endforeach()

if(mir_HAVE_ATLAS)
    ecbuild_add_test(
        TARGET            mir_tests_unit_atlas
        SOURCES           atlas.cc
        LIBS              mir
        ENVIRONMENT       ${_testEnvironment}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()

if(mir_HAVE_NETCDF)
    ecbuild_add_test(
        TARGET            mir_tests_unit_interpolation_netcdf
        SOURCES           interpolation_netcdf.cc
        LIBS              mir
        ENVIRONMENT       ${_testEnvironment}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()

if(ENABLE_BUILD_TOOLS)
    foreach(_t MIR-637.grib2)
        ecbuild_add_test(
            TARGET            mir_tests_unit_grib_fail_decode_${_t}
            COMMAND           mir-get-data
            ARGS              --dryrun ${_t}
            ENVIRONMENT       ${_testEnvironment}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
        set_tests_properties(mir_tests_unit_grib_fail_decode_${_t} PROPERTIES WILL_FAIL TRUE)
    endforeach()
endif()

