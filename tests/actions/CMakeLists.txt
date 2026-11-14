# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

# a test per action (or family of actions), named after the (first) action type
set(TESTS
    calc.formula
    crop.area
    filter.add-random
    filter.adjust-winds-directions
    filter.adjust-winds-scale-cos-latitude
    filter.bitmap
    filter.frame
    filter.globalise
    filter.limiter
    filter.mask-lsm
    filter.nabla
    filter.sh-bandpass
    filter.sh-cesaro-summation-filter
    filter.sh-truncate
    filter.statistics
    filter.unstructured
    interpolate.grid2griddef
    interpolate.grid2gridspec
    interpolate.grid2namedgrid
    interpolate.grid2octahedral-gg
    interpolate.grid2points
    interpolate.grid2reduced-gg
    interpolate.grid2reduced-gg-pl-given
    interpolate.grid2reduced-ll
    interpolate.grid2reduced-ll-pl-given
    interpolate.grid2regular-gg
    interpolate.grid2regular-ll
    interpolate.grid2rotated-namedgrid
    interpolate.grid2rotated-octahedral-gg
    interpolate.grid2rotated-reduced-gg
    interpolate.grid2rotated-reduced-gg-pl-given
    interpolate.grid2rotated-regular-gg
    interpolate.grid2rotated-regular-ll
    interpolate.grid2typedgrid
    mask.area
    misc.checkerboard
    misc.pattern
    misc.reset-missing-values
    noop
    select.field
    set.metadata
    transform.sh-to-griddef
    transform.sh-to-gridspec
    transform.sh-to-namedgrid
    transform.sh-to-octahedral-gg
    transform.sh-to-points
    transform.sh-to-reduced-gg
    transform.sh-to-reduced-gg-pl-given
    transform.sh-to-regular-gg
    transform.sh-to-regular-ll
    transform.sh-to-rotated-namedgrid
    transform.sh-to-rotated-octahedral-gg
    transform.sh-to-rotated-reduced-gg
    transform.sh-to-rotated-reduced-gg-pl-given
    transform.sh-to-rotated-regular-gg
    transform.sh-to-rotated-regular-ll
    transform.sh-to-typedgrid
    transform.sh-vod-to-UV)

foreach(_t ${TESTS})
    string(REGEX REPLACE "[.-]" "_" _name ${_t})
    ecbuild_add_test(
        TARGET            mir_tests_action_${_name}
        SOURCES           ${_t}.cc
        LIBS              mir
        ENVIRONMENT       ${_testEnvironment}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../data
    )
endforeach()
