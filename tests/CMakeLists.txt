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

# To add a test, just add the mir request with a .req extension
# and make sure the target=testdata which automatically gets substituted to testdata.prod and testdata.test

#list( APPEND _test_reqs
#  foo.grib
#  bar.grib
#)

#foreach( t ${_test_reqs} )
#        list( APPEND _test_data )
#endforeach()

#ecbuild_get_test_multidata( TARGET mit_get_test_data NAMES ${_test_data} )

#ecbuild_add_resources( TARGET tests_run_mars SOURCES_PACK ${_test_reqs} )

#foreach( _req ${_test_reqs} )

#        string(REGEX REPLACE "[^A-Za-z0-9]" "_" _tst ${_req})

#        ecbuild_add_test(
#                                TARGET test_${_tst}
#                                TYPE SCRIPT
#                                COMMAND ${CMAKE_CURRENT_BINARY_DIR}/run_mars.sh
#                                ARGS ${CMAKE_CURRENT_SOURCE_DIR}/${_req}
#        ENVIRONMENT
#          TESTS_DIR=${CMAKE_CURRENT_BINARY_DIR}
#          MARS_HOME=${PROJECT_SOURCE_DIR}
#          MARS_LSM_PATH=${libemos_BASE_DIR}/share/libemos/tables/interpol/
#                                TEST_DEPENDS get_data
#                                )

#endforeach()
