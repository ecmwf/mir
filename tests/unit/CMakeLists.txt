
# Setup cache directory
set( TEST_ECKIT_CACHE_DIR  ${CMAKE_CURRENT_BINARY_DIR}/cache/ )
file( REMOVE_RECURSE ${TEST_ECKIT_CACHE_DIR} )


foreach( _t
        test_spectral_order
        test_increments
        )
  ecbuild_add_test(
      TARGET       ${_t}
      SOURCES      ${_t}.cc
      LIBS    mir
      ENVIRONMENT
        "MIR_DEBUG=1"
        "MIR_HOME=${CMAKE_BINARY_DIR}"
        "TEST_ECKIT_CACHE_DIR=${TEST_ECKIT_CACHE_DIR}"
        "ECCODES_DEFINITION_PATH=${eccodes_BASE_DIR}/share/${grib_handling_pkg}/definitions"
        "ECCODES_SAMPLES_PATH=${eccodes_BASE_DIR}/share/${grib_handling_pkg}/samples" )
endforeach()

