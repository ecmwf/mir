# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

# a test per family of representations, of the points (canonical grids), of the grid keys and of gridSpec, on synthetic grids
set(TESTS
    canonical_grid
    gaussian
    gaussian.reduced
    gaussian.regular
    grid
    gridspec
    healpix
    irregular_latlon
    latlon
    latlon.reduced
    latlon.regular
    latlon.rotated
    regular_xy
    regular_xy.space_view
    representation
    spectral
    unstructured)

if(mir_HAVE_ECKIT_CODEC)
    list(APPEND TESTS unstructured.orca-icon-fesom)
endif()

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
