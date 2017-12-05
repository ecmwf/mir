
# Setup cache directory
set( TEST_ECKIT_CACHE_DIR  ${CMAKE_CURRENT_BINARY_DIR}/cache/ )
file( REMOVE_RECURSE ${TEST_ECKIT_CACHE_DIR} )


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

if(HAVE_NETCDF)
   list(APPEND TEST_INPUT_FILES "nemo.nc")
endif()

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


# Set testing environment
if( NOT eccodes_BASE_DIR )
    if( TARGET "grib_compare" )
        get_target_property( CMD_GRIB_COMPARE "grib_compare" LOCATION )
    endif()
    if( NOT CMD_GRIB_COMPARE )
        find_program( CMD_GRIB_COMPARE "grib_compare" )
    endif()
    if( CMD_GRIB_COMPARE )
        get_filename_component( _grib_compare_dir "${CMD_GRIB_COMPARE}" PATH )
        get_filename_component( eccodes_BASE_DIR "${_grib_compare_dir}" ABSOLUTE )
        set( eccodes_BASE_DIR "${eccodes_BASE_DIR}/../" )
    else()
        ecbuild_critical("Could not guess eccodes_BASE_DIR, nor find grib_compare (in bundle nor in path) as an alternative" )
    endif()
endif()
set( eccodes_environment
    ECCODES_DEFINITION_PATH=${eccodes_BASE_DIR}/share/${grib_handling_pkg}/definitions
    ECCODES_SAMPLES_PATH=${eccodes_BASE_DIR}/share/${grib_handling_pkg}/samples )


# Unit tests: simple interpolations
foreach(_t ${_test_reqs})
  separate_arguments(_t)
  list(GET _t -1 _out)
  string(REGEX REPLACE "[.]grib$" "" _label ${_out})

  set(_condition "")
  list(GET _t -2 _in)
  if( _in MATCHES ".nc$" )
      set(_condition "HAVE_TESSELATION")
  endif()

  ecbuild_add_test(
      TARGET       ${_label}
      COMMAND      $<TARGET_FILE:mir-tool>
      ARGS         "--caching=0" ${_t}
      TEST_DEPENDS download_mir_tests_files
      CONDITION    ${_condition}
      ENVIRONMENT
        "MIR_DEBUG=1"
        "MIR_HOME=${CMAKE_BINARY_DIR}"
        "TEST_ECKIT_CACHE_DIR=${TEST_ECKIT_CACHE_DIR}"
        ${eccodes_environment} )

endforeach()


## Unit tests: other
#foreach(_t "" "")
#  ecbuild_add_test(
#      TARGET   ${_t}
#      SOURCES "${_t}.cc"
#      LIBS    mir
#      ENVIRONMENT
#        "MIR_DEBUG=1"
#        "MIR_HOME=${CMAKE_BINARY_DIR}"  )
#endforeach()

add_subdirectory(assertions)
add_subdirectory(plans)
add_subdirectory(unit)

