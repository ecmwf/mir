# define the grib environment

set( TEST_ECKIT_CACHE_DIR  ${CMAKE_CURRENT_BINARY_DIR}/cache/ )

file( REMOVE_RECURSE ${TEST_ECKIT_CACHE_DIR} )

get_target_property( MIR_TOOL mir_tool LOCATION )

########################################################################################################################
#### TESTS

# each line is a request,
# we parse the line and take the last entry as the file to download

list( APPEND _test_reqs
  "--grid=2/2 --area=40/20/20/40 ll11_msl.grib"
  "--grid=1/1 ll22_msl.grib"
  "ll33_msl.grib"
)

foreach( _t ${_test_reqs} )
  separate_arguments(_t)
  list( GET       _t -1 _f )
  list( APPEND _test_data ${_f} )
endforeach()

ecbuild_get_test_multidata( TARGET mir_get_test_data NAMES ${_test_data} )

foreach( _t ${_test_reqs} )
  separate_arguments(_t)
  list( GET _t -1 _file )
  string(REGEX REPLACE "[^A-Za-z0-9]" "_" _tst ${_file})

  ecbuild_add_test( TARGET test_${_tst}
                    COMMAND ${MIR_TOOL}
                    ARGS ${_t} ${_file}.test
                    ENVIRONMENT
                      TEST_ECKIT_CACHE_DIR=${TEST_ECKIT_CACHE_DIR}
                      GRIB_DEFINITION_PATH=${GRIB_API_DEFINITION_PATH}
                      GRIB_SAMPLES_PATH=${GRIB_API_SAMPLES_PATH}
                    TEST_DEPENDS mir_get_test_data
                  )
endforeach()


