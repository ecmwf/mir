# Helper functions to abstract interpolation tools/environments


# Set testing environment
if( NOT grib_api_BASE_DIR )
  unset( CMD_GRIB_COMPARE )

  if( TARGET grib_compare )
    get_target_property( CMD_GRIB_COMPARE grib_compare LOCATION )
  endif()
  if( NOT CMD_GRIB_COMPARE )
    find_program( CMD_GRIB_COMPARE grib_compare NO_DEFAULT_PATH HINTS "${grib_api_BASE_DIR}" PATH_SUFFIXES "bin" )
  endif()
  if( NOT CMD_GRIB_COMPARE )
    find_program( CMD_GRIB_COMPARE grib_compare )
  endif()

  if( CMD_GRIB_COMPARE )
    get_filename_component( _grib_compare_dir "${CMD_GRIB_COMPARE}" PATH )
    get_filename_component( grib_api_BASE_DIR "${_grib_compare_dir}" ABSOLUTE )
    set( grib_api_BASE_DIR "${grib_api_BASE_DIR}/../" )
  endif()

  unset( CMD_GRIB_COMPARE )
endif()

unset( _grib_environment )
if( grib_api_BASE_DIR )
  set( grib_handling_pkg grib_api )
  if( HAVE_ECCODES )
    set( grib_handling_pkg eccodes )
  endif()
  set( _grib_environment
    GRIB_DEFINITION_PATH=${grib_api_BASE_DIR}/share/${grib_handling_pkg}/definitions
    GRIB_SAMPLES_PATH=${grib_api_BASE_DIR}/share/${grib_handling_pkg}/samples )
endif()
ecbuild_info( "Test _grib_environment: ${_grib_environment}" )


# Easily add interpolation tests
# (optional arg #5: test dependencies, default none)
# (optional arg #6: environment variables, default none additional)
function( interpolation_add_test_interpol
        _label
        _file1
        _file2
        _options )
    set( _depends     "" )
    set( _environment "MIRHOME=${CMAKE_BINARY_DIR}" )
    if( ${ARGC} GREATER 4 )
        set( _depends "${ARGV4}" )
    endif()
    if( ${ARGC} GREATER 5 )
        set( _environment "${ARGV5}" )
    endif()
    ecbuild_add_test(
        TARGET       ${_label}_interpol
        TEST_DEPENDS ${_depends}
        DEPENDS      mir_tool
        COMMAND      ${CMAKE_BINARY_DIR}/bin/mir_tool
        ARGS         ${_options} "${_file1}" "${_file2}"
        ENVIRONMENT  ${_environment} ${_grib_environment} )
endfunction()


# Easily add comparison-to-reference results tests
# (optional arg #4: test dependencies, default none)
function( interpolation_add_test_compare
        _label
        _file1
        _file2 )
    set( _depends "" )
    if( ${ARGC} GREATER 3 )
        set( _depends "${ARGV3}" )
    endif()

    # relax comparisons across platforms/compiler (FIXME: temporary)
    unset( _tolerate )
    list( APPEND _tolerate "--percent=0.1" )

    if(    (_label MATCHES "(F640|N640|O1280)_to_hirlam_non-rotated_F80")
        OR (_label MATCHES "N640_to_F80")
        OR (_label MATCHES "O1280_to_rotated_regular_ll")
        OR (_label MATCHES "regular_ll_to_hirlam_non-rotated_(F48|F80)") )

        # within 2x packing tolerance
        list(APPEND _tolerate "--packing=2" )

    endif()
    ecbuild_add_test(
        TARGET       ${_label}_compare
        TEST_DEPENDS ${_label}_interpol ${_depends}
        DEPENDS      mir_compare
        COMMAND      ${CMAKE_BINARY_DIR}/bin/mir_compare
        ARGS         ${_tolerate} "${_file1}" "${_file2}"
        ENVIRONMENT  ${_grib_environment} )
endfunction()

