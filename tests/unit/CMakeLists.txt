
# Setup cache directory
set( TEST_ECKIT_CACHE_DIR  ${CMAKE_CURRENT_BINARY_DIR}/cache/ )
file( REMOVE_RECURSE ${TEST_ECKIT_CACHE_DIR} )

set(tests
    bounding_box
    grib_encoding
    increments
    rotations
    spectral_order
    )
if(HAVE_ATLAS)
    list(APPEND tests atlas)
endif()

set( _environment
    "MIR_DEBUG=1"
    "MIR_HOME=${CMAKE_BINARY_DIR}"
    "TEST_ECKIT_CACHE_DIR=${TEST_ECKIT_CACHE_DIR}"
    "ECCODES_DEFINITION_PATH=${eccodes_BASE_DIR}/share/eccodes/definitions"
    "ECCODES_SAMPLES_PATH=${eccodes_BASE_DIR}/share/eccodes/samples" )

foreach( _t ${tests})
  ecbuild_add_test(
      TARGET      mir_tests_unit_${_t}
      SOURCES     ${_t}.cc
      LIBS        mir
      ENVIRONMENT ${_environment} )
endforeach()

