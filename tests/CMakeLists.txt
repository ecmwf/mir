
# Setup cache directory
set( TEST_ECKIT_CACHE_DIR  ${CMAKE_CURRENT_BINARY_DIR}/cache/ )
file( REMOVE_RECURSE ${TEST_ECKIT_CACHE_DIR} )


# Test settings and helper functions
include("${PROJECT_SOURCE_DIR}/tests/settings.cmake")


# Set interpolation tests
# (each line is a request, the previous-to-last entry in each line is a file to download)
unset(_tests_reqs)
list( APPEND _test_reqs
  "--grid=1/1 --area=40/20/20/40 ll11_msl.grib ll11_msl.grib.test"
                     "--grid=2/2 ll22_msl.grib ll22_msl.grib.test"
                                "ll33_msl.grib ll33_msl.grib.test" )

unset(_tests_src_files)
foreach( _t ${_test_reqs} )
  separate_arguments(_t)
  list( GET    _t -2 _file )
  list( APPEND _tests_src_files ${_file} )
endforeach()
list(REMOVE_DUPLICATES _tests_src_files)

ecbuild_get_test_multidata(
    TARGET download_tests_src_files
    NAMES  ${_tests_src_files} )


# Unit tests: simple interpolations
foreach( _t ${_test_reqs} )
  separate_arguments(_t)
  list( GET _t -2 _file )

  string(REGEX REPLACE "[^A-Za-z0-9]" "_" _label ${_file})
  set(_label "test_${_label}_interpol")

  ecbuild_add_test(
      TARGET       ${_label}
      COMMAND      mir_tool
      ARGS         ${_t}
      TEST_DEPENDS download_tests_src_files
      ENVIRONMENT
        "MIRHOME=${CMAKE_BINARY_DIR}"
        "TEST_ECKIT_CACHE_DIR=${TEST_ECKIT_CACHE_DIR}"
        ${_grib_environment} )

endforeach()


# Regression tests
if( HAVE_MIR_TESTS_REGRESS )
  add_subdirectory("regress")
endif()

