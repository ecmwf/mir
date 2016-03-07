# Helper functions to abstract interpolation tools/environments


# Easily add interpolation tests
# (optional arg #5: test dependencies, default none)
function( interpolation_add_test_interpol
        _label
        _file1
        _file2
        _options )
    set( _depends     "" )
#   set( _condition   "" )
    if( ${ARGC} GREATER 4 )
        set( _depends "${ARGV4}" )
    endif()
    if( ${ARGC} GREATER 5 )
#       set( _condition "${ARGV5}" )
    endif()
    ecbuild_add_test(
        TARGET       ${_label}_interpol
        TEST_DEPENDS ${_depends}
#       CONDITION    ${_condition}
        DEPENDS      mir_tool
        COMMAND      ${CMAKE_BINARY_DIR}/bin/mir_tool
        ARGS         ${_options} "${_file1}" "${_file2}" )
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
    set( _tolerate "" )
    if( FALSE )
        set( _tolerate "--relative=1" )  #FIXME
    endif()
    ecbuild_add_test(
        TARGET       ${_label}_compare
        TEST_DEPENDS ${_label}_interpol ${_depends}
        DEPENDS      mir_compare
        COMMAND      ${CMAKE_BINARY_DIR}/bin/mir_compare
        ARGS         ${_tolerate} "${_file1}" "${_file2}" )
endfunction()

