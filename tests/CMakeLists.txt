find_program( GRIB_COMPARE grib_compare )
find_program( GRIB_LS      grib_ls )

if( NOT GRIB_COMPARE AND TARGET grib_compare )
	get_target_property( GRIB_COMPARE grib_compare LOCATION )
        get_target_property( GRIB_LS      grib_ls      LOCATION )
endif()

# define the grib environment

set( GRIB_DEFINITION_PATH ${grib_api_BASE_DIR}/share/grib_api/definitions/ )
set( GRIB_SAMPLES_PATH    ${grib_api_BASE_DIR}/share/grib_api/samples/ )




#### TESTS

get_target_property( MIR_TOOL mir_tool LOCATION )

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
                      GRIB_DEFINITION_PATH=${GRIB_DEFINITION_PATH}
                      GRIB_SAMPLES_PATH=${GRIB_SAMPLES_PATH}
                    TEST_DEPENDS mir_get_test_data
                  )
endforeach()

#### TEST ALL GRIB_API templates

# debug_var(GRIB_SAMPLES_PATH)
file(GLOB_RECURSE grib_tmpl  ${GRIB_SAMPLES_PATH}*.tmpl)

foreach( _t ${grib_tmpl} )
  get_filename_component(_tst ${_t} NAME_WE)
  # debug_var(_tst)
  if(${_tst} STREQUAL "budg")
    # Ignore
  else()
  ecbuild_add_test( TARGET test_grib_sample_${_tst}
                    COMMAND ${MIR_TOOL}
                    ARGS "--grid=1/1" ${_t} /dev/null
                    ENVIRONMENT
                      GRIB_DEFINITION_PATH=${GRIB_DEFINITION_PATH}
                      GRIB_SAMPLES_PATH=${GRIB_SAMPLES_PATH}
                  )
endif()
endforeach()
# debug_var(grib_tmpl )
