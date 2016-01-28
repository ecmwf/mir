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

########################################################################################################################
#### TEST ALL GRIB_API templates

file(GLOB_RECURSE testGribTemplates ${GRIB_API_SAMPLES_PATH}/*.tmpl)

# ecbuild_debug_var(GRIB_SAMPLES_PATH)
# ecbuild_debug_var(testGribTemplates)


# skip these samples

list( APPEND skipTmpl
    ".*budg.tmpl"
    ".*polar_stereographic_pl_grib.*"
    ".*polar_stereographic_sfc_grib.*"
    ".*reduced_gg_pl_1280_grib.*"
    ".*reduced_gg_pl_2000_grib.*"
    ".*reduced_ll_sfc_grib.*"
    ".*reduced_rotated_gg_pl_1280_grib.*"
    ".*reduced_rotated_gg_pl_2000_grib.*"
    ".*rotated_ll_pl_grib.*"
    ".*rotated_ll_sfc_grib.*"
)

ecbuild_list_exclude_pattern( LIST testGribTemplates REGEX ${skipTmpl} )

# ecbuild_debug_var(testGribTemplates)

foreach( _t ${testGribTemplates} )

    get_filename_component(_tst ${_t} NAME_WE)

    ecbuild_add_test( TARGET test_grib_sample_${_tst}
                      COMMAND ${MIR_TOOL}
                      ARGS "--grid=1/1" ${_t} /dev/null
                      ENVIRONMENT
                        TEST_ECKIT_CACHE_DIR=${TEST_ECKIT_CACHE_DIR}
                        GRIB_DEFINITION_PATH=${GRIB_API_DEFINITION_PATH}
                        GRIB_SAMPLES_PATH=${GRIB_API_SAMPLES_PATH}
                    )

endforeach()
