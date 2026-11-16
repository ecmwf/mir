# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

# a test per family of representations, of iterators, of the grid keys and of gridSpec, on synthetic grids
set(TESTS
    gaussian
    grid
    gridspec
    healpix
    iterator
    latlon
    regular_xy
    representation
    spectral
    unstructured)

foreach(_t ${TESTS})
    string(REGEX REPLACE "[.-]" "_" _name ${_t})
    ecbuild_add_test(
        TARGET            mir_tests_repres_${_name}
        SOURCES           ${_t}.cc
        LIBS              mir
        ENVIRONMENT       ${_testEnvironment}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../data
    )
endforeach()
