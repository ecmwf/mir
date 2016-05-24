
# Setup cache directory
set( TEST_ECKIT_CACHE_DIR  ${CMAKE_CURRENT_BINARY_DIR}/cache/ )
file( REMOVE_RECURSE ${TEST_ECKIT_CACHE_DIR} )


# Test settings and helper functions
include("${PROJECT_SOURCE_DIR}/tests/settings.cmake")


# download interpolation source files
# (each line is a request, the last entry in each line is a file to download)
list( APPEND _test_reqs
  "--grid=1/1 --area=40/20/20/40 ll11_msl.grib"
                     "--grid=2/2 ll22_msl.grib"
                                "ll33_msl.grib" )

unset(_tests_src_files)
foreach( _t ${_test_reqs} )
  separate_arguments(_t)
  list( GET    _t -1 _file )
  list( APPEND _tests_src_files ${_file} )
endforeach()

ecbuild_get_test_multidata(
    TARGET download_tests_src_files
    NAMES  ${_tests_src_files} )


# Unit tests: simple interpolations
foreach( _t ${_test_reqs} )
  separate_arguments(_t)
  list( GET _t -1 _file )

  string(REGEX REPLACE "[^A-Za-z0-9]" "_" _label ${_file})
  set(_label "test_${_label}_interpol")

  ecbuild_add_test(
      TARGET       ${_label}
      COMMAND      mir_tool
      ARGS         ${_t} "${_file}.test"
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

