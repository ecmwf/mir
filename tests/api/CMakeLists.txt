# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

set(TESTS
    input
    input.artificial
    input.empty
    input.geopoints
    input.grib
    input.multi-dimensional
    input.multi-stream
    input.vector
    job
    output
    output.empty
    output.geopoints
    output.gmsh
    output.grib
    output.multi-dimensional
    output.values
    output.vector
    parametrisation.cached
    parametrisation.combined
    parametrisation.configuration-wrapper
    parametrisation.default
    parametrisation.gridspec
    parametrisation.runtime
    parametrisation.same
    parametrisation.simple
    parametrisation.spec
    raw_memory)

if(mir_HAVE_NETCDF)
    list(APPEND TESTS input.netcdf)
endif()

foreach(_t ${TESTS})
    string(REGEX REPLACE "[.-]" "_" _name ${_t})
    ecbuild_add_test(
        TARGET            mir_tests_api_${_name}
        SOURCES           ${_t}.cc
        LIBS              mir
        ENVIRONMENT       ${_testEnvironment}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../data
    )
endforeach()
