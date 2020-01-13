# ecbuild_find_python( VERSION 2.7 )

# set(MIR_TOOL     "$<TARGET_FILE:mir-tool>")
# set(MIR_COMPARE  "$<TARGET_FILE:mir-compare>")
# set(GRIB_GET     "$<TARGET_FILE:grib_get>")
# set(GRIB_COMPARE "$<TARGET_FILE:grib_compare>")

get_target_property( MIR_TOOL      "mir-tool"      LOCATION )

find_program( DIFF_TOOL NAMES "diff" "cmp" "false" )
find_program( MARS_SCRIPT NAMES "mars" "false" )

configure_file( mir-test.sh.in mir-test.sh @ONLY )

set( _environment
    "MIR_DEBUG=1"
    "MIR_HOME=${CMAKE_BINARY_DIR}"
    "TEST_ECKIT_CACHE_DIR=${TEST_ECKIT_CACHE_DIR}")
if( NOT HAVE_MEMFS )
    list( APPEND _environment 
        "ECCODES_DEFINITION_PATH=${eccodes_BASE_DIR}/share/eccodes/definitions"
        "ECCODES_SAMPLES_PATH=${eccodes_BASE_DIR}/share/eccodes/samples" )
endif()

file(GLOB_RECURSE test_files LIST_DIRECTORIES false *.test)

foreach( _t ${test_files} )

  ecbuild_add_test(
      TARGET      ${_t}
      CONDITION   MARS_SCRIPT AND HAVE_TEST_ASSERTIONS
      COMMAND     mir-test.sh
      ARGS        ${_t}
      ENVIRONMENT ${_environment} )

endforeach()
