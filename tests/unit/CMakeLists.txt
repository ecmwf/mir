
# Setup cache directory
set(TEST_ECKIT_CACHE_DIR ${CMAKE_CURRENT_BINARY_DIR}/cache/)
file(REMOVE_RECURSE ${TEST_ECKIT_CACHE_DIR})

foreach(_t
#    MIR-333
    MIR-425
    action_graph
    increments
    iterator
    spectral_order
    wind)
    ecbuild_add_test(
        TARGET            mir_tests_unit_${_t}
        SOURCES           ${_t}.cc
        LIBS              mir
        ENVIRONMENT       ${_testEnvironment}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endforeach()

foreach(_t
    bounding_box
    gaussian_grid
    grib_encoding
    style)
    ecbuild_add_test(
        TARGET            mir_tests_unit_${_t}
        SOURCES           ${_t}.cc
        LIBS              mir
        ENVIRONMENT       ${_testEnvironment}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    if(NOT mir_HAVE_ATLAS)
        set_tests_properties(mir_tests_unit_${_t} PROPERTIES DISABLED TRUE)
    endif()
endforeach()

if(mir_HAVE_ATLAS)
    ecbuild_add_test(
        TARGET            mir_tests_unit_atlas
        SOURCES           atlas.cc
        LIBS              mir
        ENVIRONMENT       ${_testEnvironment}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()

