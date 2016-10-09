# ecbuild_find_python( VERSION 2.7 )

file(GLOB_RECURSE test_files LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_BINARY_DIR} *.test)

# set(MIR_TOOL     "$<TARGET_FILE:mir-tool>")
# set(MIR_COMPARE  "$<TARGET_FILE:mir-compare>")
# set(GRIB_GET     "$<TARGET_FILE:grib_get>")
# set(GRIB_COMPARE "$<TARGET_FILE:grib_compare>")

get_target_property( MIR_TOOL      "mir-tool"      LOCATION )
get_target_property( MIR_COMPARE   "mir-compare"   LOCATION )
get_target_property( GRIB_GET      "grib_get"      LOCATION )
get_target_property( GRIB_COMPARE  "grib_compare"  LOCATION )

configure_file( mir-test.sh.in mir-test.sh @ONLY )

set( eccodes_environment
    ECCODES_DEFINITION_PATH=${eccodes_BASE_DIR}/share/${grib_handling_pkg}/definitions
    ECCODES_SAMPLES_PATH=${eccodes_BASE_DIR}/share/${grib_handling_pkg}/samples )

ecbuild_info(${test_files})

foreach( _t ${test_files} )

  ecbuild_add_test(
      # TYPE       PYTHON

      COMMAND    mir-test.sh

      ARGS       ${_t}

    ENVIRONMENT
        "MIR_DEBUG=1"
        "MIR_HOME=${CMAKE_BINARY_DIR}"
        "TEST_ECKIT_CACHE_DIR=${TEST_ECKIT_CACHE_DIR}"
        ${eccodes_environment}
  )

endforeach()
