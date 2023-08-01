
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

