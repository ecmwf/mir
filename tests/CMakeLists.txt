find_program( GRIB_COMPARE grib_compare )
find_program( GRIB_LS      grib_ls )

if( NOT GRIB_COMPARE AND TARGET grib_compare )
	get_target_property( GRIB_COMPARE grib_compare LOCATION )
        get_target_property( GRIB_LS      grib_ls      LOCATION )
endif()

# define the grib environment

set( GRIB_DEFINITION_PATH ${grib_api_BASE_DIR}/share/grib_api/definitions/ )
set( GRIB_SAMPLES_PATH    ${grib_api_BASE_DIR}/share/grib_api/samples/ )

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
                      GRIB_DEFINITION_PATH=${GRIB_DEFINITION_PATH}
                      GRIB_SAMPLES_PATH=${GRIB_SAMPLES_PATH}
                    TEST_DEPENDS mir_get_test_data
                  )
endforeach()

########################################################################################################################
#### TEST ALL GRIB_API templates

file(GLOB_RECURSE grib_tmpl ${GRIB_SAMPLES_PATH}*.tmpl)

#debug_var(GRIB_SAMPLES_PATH)
#debug_var(grib_tmpl)


# skip these samples

list( APPEND skipTmpl
    ".*budg.tmpl"
    ".*reduced_gg_pl_1280_grib1.tmpl"
    ".*reduced_gg_pl_1280_grib2.tmpl"
    ".*reduced_rotated_gg_pl_1280_grib1.tmpl"
    ".*reduced_rotated_gg_pl_1280_grib2.tmpl"
    ".*reduced_gg_pl_2000_grib1.tmpl"
    ".*reduced_gg_pl_2000_grib2.tmpl"
    ".*reduced_rotated_gg_pl_2000_grib1.tmpl"
    ".*reduced_rotated_gg_pl_2000_grib2.tmpl"
    ".*polar_stereographic_sfc_grib1.tmpl"
    ".*polar_stereographic_sfc_grib2.tmpl"
)

ecbuild_list_remove_pattern( "${grib_tmpl}" "${skipTmpl}" testGribTemplates )

debug_var(testGribTemplates)

foreach( _t ${testGribTemplates} )

    get_filename_component(_tst ${_t} NAME_WE)

    ecbuild_add_test( TARGET test_grib_sample_${_tst}
                      COMMAND ${MIR_TOOL}
                      ARGS "--grid=1/1" ${_t} /dev/null
                      ENVIRONMENT
                        TEST_ECKIT_CACHE_DIR=${TEST_ECKIT_CACHE_DIR}
                        GRIB_DEFINITION_PATH=${GRIB_DEFINITION_PATH}
                        GRIB_SAMPLES_PATH=${GRIB_SAMPLES_PATH}
                    )

endforeach()
