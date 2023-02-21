
# Setup cache directory
set(TEST_ECKIT_CACHE_DIR ${CMAKE_CURRENT_BINARY_DIR}/cache/)
file(REMOVE_RECURSE ${TEST_ECKIT_CACHE_DIR})

foreach(_t
#    MIR-333
#    MIR-356
    MIR-425
    MIR-536
    PGEN-412
    area
    action_graph
    bounding_box
    gaussian_grid
    formula
    grib_basic_angle
    grib_encoding
    grid_box_method
    increments
    input_MultiDimensionalInput
    interpolations
    iterator
    knn_weighting
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

