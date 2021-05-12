
# Setup cache directory
set(TEST_ECKIT_CACHE_DIR ${CMAKE_CURRENT_BINARY_DIR}/cache/)
file(REMOVE_RECURSE ${TEST_ECKIT_CACHE_DIR})


# Set list of input data files
set(TEST_INPUT_FILES
    "2t_O640.grib"
    "msl_N640.grib"
    "msl_regular_ll_1-1.grib"
    "msl_regular_ll_2-2.grib"
    "msl_regular_ll_3-3.grib"
    "q_F640.grib"
    "vo-d_sh.grib"
    "z_sh.grib")

if(mir_HAVE_NETCDF AND atlas_HAVE_TESSELATION)
   list(APPEND TEST_INPUT_FILES "nemo.nc")
endif()

# Set interpolation tests
# (each line is an interpolation, the previous-to-last entry in each line is a file to download)
unset(_test_reqs)
foreach(_in ${TEST_INPUT_FILES})
    foreach(_pproc
            ""
            "--grid=1/1 --area=40/20/20/40"
            "--grid=1/1 --area=40/20/20/40 --frame=2"
            "--grid=2/2"
            "--grid=3/3"
            "--rotation=-90/0"
            "--rotation=-89/1")
        if("${_pproc}" STREQUAL "")
            set(_label "mir_tests_nointerpol ${_in}")
        else()
            set(_label "mir_tests_interpol ${_in} to ${_pproc}")
        endif()
        string(REGEX REPLACE "[.]grib"      ""  _label ${_label})
        string(REGEX REPLACE "[^A-Za-z0-9]" "_" _label ${_label})
        string(REGEX REPLACE "[ _]+"        "_" _label ${_label})

        if(NOT (_in MATCHES ".*_sh[.]grib$" AND _pproc MATCHES "--frame=.*" OR
                _in MATCHES ".*[.]nc$" AND _pproc MATCHES "^$"))
            if(_in MATCHES ".*[.]nc$")
                set(_pproc "${_pproc} --interpolation=linear")
            endif()
            set(_out "${_label}.grib")
            list(APPEND _test_reqs "${_pproc} ${_in} ${_out}")
        endif()
    endforeach()
endforeach()

unset(_tests_src_files)
foreach(_t ${_test_reqs})
    separate_arguments(_t)
    list(GET _t -2 _file)
    list(APPEND _tests_src_files ${_file})
endforeach()
list(REMOVE_DUPLICATES _tests_src_files)

ecbuild_get_test_multidata(TARGET download_mir_tests_files NAMES ${_tests_src_files})


# Set testing environment
set(_testEnvironment "MIR_DEBUG=1" "MIR_HOME=${CMAKE_BINARY_DIR}" "TEST_ECKIT_CACHE_DIR=${TEST_ECKIT_CACHE_DIR}")

if(NOT eccodes_HAVE_MEMFS)
    if(ECCODES_DEFINITION_PATH)
        list(APPEND _testEnvironment "ECCODES_DEFINITION_PATH=${ECCODES_DEFINITION_PATH}")
    endif()
    if(ECCODES_SAMPLES_PATH)
        list(APPEND _testEnvironment "ECCODES_SAMPLES_PATH=${ECCODES_SAMPLES_PATH}")
    endif()
endif()


# Unit tests: simple interpolations
foreach(_t ${_test_reqs})
    separate_arguments(_t)
    list(GET _t -1 _out)
    list(GET _t -2 _in)
    unset(extraEnvironment)
    if(_in MATCHES "[.]nc$")
        set(_extraEnvironment "MIR_CHECK_DUPLICATE_POINTS=0")
    endif()
    string(REGEX REPLACE "[.]grib$" "" _label ${_out})
    ecbuild_add_test(
        TARGET       ${_label}
        COMMAND      $<TARGET_FILE:mir-tool>
        ARGS         "--caching=0" ${_t}
        TEST_DEPENDS download_mir_tests_files
        ENVIRONMENT  ${_testEnvironment} ${_extraEnvironment})
    if(NOT mir_HAVE_ATLAS AND (_label MATCHES "_sh_to_grid_" OR _label MATCHES "[123]_to_rotation_89_1"))
        set_tests_properties(${_label} PROPERTIES DISABLED TRUE)
    endif()
endforeach()


# ecCodes really doesn't build tools, so disabling testing is required
if(eccodes_HAVE_BUILD_TOOLS)
    if (HAVE_TEST_ASSERTIONS)
        add_subdirectory(assertions)
    endif()
    add_subdirectory(tool)
else()
    message(WARNING "${PROJECT_NAME}: assertion tests (option TEST_ASSERTIONS) requires ecCodes option BUILD_TOOLS, tests disabled")
    message(WARNING "${PROJECT_NAME}: tool tests requires ecCodes option BUILD_TOOLS, tests disabled")
endif()


# defaults exercise non-core functionality
if(mir_HAVE_ATLAS)
    add_subdirectory(plans)
endif()
add_subdirectory(unit)

