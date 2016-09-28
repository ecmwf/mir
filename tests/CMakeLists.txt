
# Setup cache directory
set( TEST_ECKIT_CACHE_DIR  ${CMAKE_CURRENT_BINARY_DIR}/cache/ )
file( REMOVE_RECURSE ${TEST_ECKIT_CACHE_DIR} )


# Test settings and helper functions
include("${PROJECT_SOURCE_DIR}/tests/settings.cmake")

# Set list of input data files

set(TEST_INPUT_FILES
        "2t_O640.grib"
        "msl_N640.grib"
        "msl_regular_ll_1-1.grib"
        "msl_regular_ll_2-2.grib"
        "msl_regular_ll_3-3.grib"
        "q_F640.grib" )

# Add input data with spherical harmonic if possible
if(ATLAS_HAVE_TRANS)
    list(APPEND TEST_INPUT_FILES
        "vo-d_sh.grib"
        "z_sh.grib" )
endif(ATLAS_HAVE_TRANS)

# Set interpolation tests
# (each line is an interpolation, the previous-to-last entry in each line is a file to download)
unset(_test_reqs)
foreach( _in ${TEST_INPUT_FILES})
  foreach( _pproc
          ""
          "--grid=1/1 --area=40/20/20/40"
          "--grid=1/1 --area=40/20/20/40 --frame=2"
          "--grid=2/2"
          "--grid=3/3"
          "--rotation=-90/0"
          "--rotation=-89/1" )
      if( "${_pproc}" STREQUAL "" )
        set(_label "mir_tests_nointerpol ${_in}")
      else()
        set(_label "mir_tests_interpol ${_in} to ${_pproc}")
      endif()
      string(REGEX REPLACE "[.]grib"      ""  _label ${_label})
      string(REGEX REPLACE "[^A-Za-z0-9]" "_" _label ${_label})
      string(REGEX REPLACE "[ _]+"        "_" _label ${_label})
      if( NOT (_in MATCHES ".*_sh.grib$" AND _pproc MATCHES "--frame=.*") )

        set(_out "${_label}.grib")
        list(APPEND _test_reqs "${_pproc} ${_in} ${_out}")

      endif()
  endforeach()
endforeach()

unset(_tests_src_files)
foreach(_t ${_test_reqs})
  separate_arguments(_t)
  list(GET    _t -2 _file)
  list(APPEND _tests_src_files ${_file})
endforeach()
list(REMOVE_DUPLICATES _tests_src_files)

ecbuild_get_test_multidata(
    TARGET download_mir_tests_files
    NAMES  ${_tests_src_files} )


# Unit tests: simple interpolations
foreach(_t ${_test_reqs})
  separate_arguments(_t)
  list(GET _t -1 _out)
  string(REGEX REPLACE "[.]grib$" "" _label ${_out})

  ecbuild_add_test(
      TARGET       ${_label}
      COMMAND      $<TARGET_FILE:mir-tool>
      ARGS         "--caching=0" ${_t}
      TEST_DEPENDS download_mir_tests_files
      ENVIRONMENT
        "MIRHOME=${CMAKE_BINARY_DIR}"
        "TEST_ECKIT_CACHE_DIR=${TEST_ECKIT_CACHE_DIR}"
        ${_grib_environment} )

endforeach()


# Unit tests: other
foreach(_t "mir_tests_RotateGrid")
  ecbuild_add_test(
      TARGET   ${_t}
      SOURCES "${_t}.cc"
      LIBS    mir
      ENVIRONMENT
        "MIR_TRACE=1"
        "MIRHOME=${CMAKE_BINARY_DIR}"  )
endforeach()


# Regression tests
if( HAVE_MIR_TESTS_REGRESS )
  add_subdirectory("regress")
endif()

# if( HAVE_MIR_TESTS_REGRESS )
  add_subdirectory(assertions)
# endif()

