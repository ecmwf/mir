# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

set(TESTS
    comparator
    counter
    distribution
    field
    method
    mode
    moments
    statistics)

foreach(_t ${TESTS})
    string(REGEX REPLACE "[.-]" "_" _name ${_t})
    ecbuild_add_test(
        TARGET            mir_tests_stats_${_name}
        SOURCES           ${_t}.cc
        LIBS              mir
        ENVIRONMENT       ${_testEnvironment}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../data
    )
endforeach()
