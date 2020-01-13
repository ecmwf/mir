
# Setup cache directory
set( TEST_ECKIT_CACHE_DIR  ${CMAKE_CURRENT_BINARY_DIR}/cache/ )
file( REMOVE_RECURSE ${TEST_ECKIT_CACHE_DIR} )

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
    )
if(HAVE_ATLAS)
    list(APPEND tests atlas)
endif()

set( _environment
    "MIR_DEBUG=1"
    "MIR_HOME=${CMAKE_BINARY_DIR}"
    "TEST_ECKIT_CACHE_DIR=${TEST_ECKIT_CACHE_DIR}")
if( NOT HAVE_MEMFS )
    list( APPEND _environment 
        "ECCODES_DEFINITION_PATH=${eccodes_BASE_DIR}/share/eccodes/definitions"
        "ECCODES_SAMPLES_PATH=${eccodes_BASE_DIR}/share/eccodes/samples" )
endif()

foreach( _t ${tests})
  ecbuild_add_test(
      TARGET            mir_tests_unit_${_t}
      SOURCES           ${_t}.cc
      LIBS              mir
      ENVIRONMENT       ${_environment}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
endforeach()

