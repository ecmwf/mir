
# Setup cache directory
set(TEST_ECKIT_CACHE_DIR ${CMAKE_CURRENT_BINARY_DIR}/cache/)
file(REMOVE_RECURSE ${TEST_ECKIT_CACHE_DIR})

set(tests
#    MIR-333
    MIR-425
    action_graph
    bounding_box
    gaussian_grid
    grib_encoding
    increments
    iterator
    spectral_order
    style
    wind
    atlas)

foreach(_t ${tests})
    ecbuild_add_test(
        TARGET            mir_tests_unit_${_t}
        SOURCES           ${_t}.cc
        LIBS              mir
        ENVIRONMENT       ${_testEnvironment}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endforeach()

