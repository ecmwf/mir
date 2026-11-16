# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

set(TESTS
    earthkit-geo
    finite_element
    grid_box
    interpolations
    knn
    knn_distance
    lsm
    nonlinear
    resampling
    search
    spec
    structured
    weight_matrix)

if(mir_HAVE_NETCDF)
    list(APPEND TESTS interpolation_netcdf)
endif()

foreach(_t ${TESTS})
    string(REGEX REPLACE "[.-]" "_" _name ${_t})
    ecbuild_add_test(
        TARGET            mir_tests_interpolation_${_name}
        SOURCES           ${_t}.cc
        LIBS              mir
        ENVIRONMENT       ${_testEnvironment}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../data
    )
endforeach()
