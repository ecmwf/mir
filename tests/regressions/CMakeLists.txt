# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

# <gridType>.cc: post-processing (as the mir tool) of an artificial input of the grid type (by gridSpec), to
# several targets (a CASE by target and topic)
set(CXX_TESTS
    healpix
    reduced_gg
    regular_gg
    regular_ll
    regular_xy
    rotated_ll
    unstructured_grid)

foreach(_t ${CXX_TESTS})
    ecbuild_add_test(
        TARGET            mir_tests_regressions_${_t}
        SOURCES           ${_t}.cc
        LIBS              mir
        ENVIRONMENT       ${_testEnvironment}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endforeach()

# <gridType>_to_<gridType>.<name>[.<labels>].test, labels:
# - .lsm. and .proj. need the bundled land-sea masks and PROJ (otherwise not added)
# - .slow. (ctest label slow), .fail. expected to fail
# - .x: with the configuration files in config/
set(TESTS
    geopoints_to_geopoints.xyv.001.test
    geopoints_to_geopoints.xyv.002.test
    lambert_azimuthal_equal_area_to_lambert_azimuthal_equal_area.area.test
    polar_stereographic_to_regular_ll.regrid.001.test
    polar_stereographic_to_regular_ll.regrid.002.test
    reduced_gg_to_healpix.grid-box-average-accuracy.test
    reduced_gg_to_lambert.regrid.test
    reduced_gg_to_lambert_azimuthal_equal_area.regrid.test
    reduced_gg_to_reduced_gg.scalar-gradient.001.test
    reduced_gg_to_reduced_gg.scalar-gradient.002.test
    reduced_gg_to_reduced_gg.uv2uv-uv-gradient.test
    reduced_gg_to_regular_gg.area-nearest-lsm.lsm.test
    reduced_gg_to_regular_ll.area-nearest-lsm.lsm.test
    reduced_gg_to_regular_ll.area.001.test
    reduced_gg_to_regular_ll.compatibility.001.test
    reduced_gg_to_regular_ll.compatibility.002.test
    reduced_gg_to_regular_ll.delete-local-definition.test
    reduced_gg_to_regular_ll.grid-box-average.test
    reduced_gg_to_regular_ll.regrid.001.test
    reduced_gg_to_regular_ll.wrongly-encoded.001.test
    reduced_gg_to_regular_ll.wrongly-encoded.002.fail.test
    reduced_gg_to_regular_ll.wrongly-encoded.003.fail.test
    reduced_gg_to_regular_ll.wrongly-encoded.004.fail.test
    reduced_gg_to_rotated_ll.area-nearest-lsm.lsm.test
    reduced_gg_to_rotated_ll.area-rotation.lsm.test
    reduced_gg_to_rotated_ll.gridspec-rotation.001.test
    reduced_gg_to_rotated_ll.gridspec-rotation.002.test
    reduced_ll_to_reduced_gg.regrid.test
    reduced_ll_to_regular_ll.accuracy.test
    reduced_ll_to_regular_ll.area-globalise.test
    reduced_ll_to_regular_ll.globalise.test
    reduced_ll_to_regular_ll.ieee.test
    reduced_ll_to_regular_ll.wrongly-encoded.fail.test
    regular_gg_to_regular_ll.linear-finite-element-projection-fail.test
    regular_ll_to_lambert.edition.test
    regular_ll_to_lambert.regrid.001.test
    regular_ll_to_lambert.regrid.002.test
    regular_ll_to_lambert.regrid.003.test
    regular_ll_to_lambert_azimuthal_equal_area.regrid.001.test
    regular_ll_to_lambert_azimuthal_equal_area.regrid.002.test
    regular_ll_to_regular_gg.bitmap-area.test
    regular_ll_to_regular_ll.accuracy-ieee.001.test
    regular_ll_to_regular_ll.accuracy-ieee.002.test
    regular_ll_to_regular_ll.accuracy-ieee.003.test
    regular_ll_to_regular_ll.accuracy.test
    regular_ll_to_regular_ll.area-so.test
    regular_ll_to_regular_ll.bitmap-area.test
    regular_ll_to_regular_ll.ccsds-grib-edition-conversion.001.test
    regular_ll_to_regular_ll.ccsds-grib-edition-conversion.002.test
    regular_ll_to_regular_ll.ccsds.test
    regular_ll_to_regular_ll.config-not-found-home.001.fail.test
    regular_ll_to_regular_ll.config-not-found-home.002.test
    regular_ll_to_regular_ll.config.x
    regular_ll_to_regular_ll.edition.test
    regular_ll_to_regular_ll.ieee-accuracy.001.test
    regular_ll_to_regular_ll.ieee-accuracy.002.test
    regular_ll_to_regular_ll.ieee-accuracy.003.test
    regular_ll_to_regular_ll.ieee-accuracy.004.test
    regular_ll_to_regular_ll.ieee.001.test
    regular_ll_to_regular_ll.ieee.002.test
    regular_ll_to_regular_ll.ieee.003.test
    regular_ll_to_regular_ll.ieee.004.test
    regular_ll_to_regular_ll.nearest-lsm-distance-weighting-with-lsm.001.lsm.test
    regular_ll_to_regular_ll.nearest-lsm-distance-weighting-with-lsm.002.lsm.test
    regular_ll_to_regular_ll.nearest-lsm.lsm.test
    regular_ll_to_regular_ll.regrid.001.test
    regular_ll_to_regular_ll.regrid.002.test
    regular_ll_to_regular_ll.simple.test
    regular_ll_to_regular_ll.so.001.test
    regular_ll_to_regular_ll.so.002.test
    regular_ll_to_regular_ll.so.003.test
    regular_ll_to_regular_ll.so.004.test
    regular_ll_to_regular_ll.so.005.test
    regular_ll_to_regular_ll.so.006.test
    regular_ll_to_regular_ll.wrongly-encoded.fail.test
    rotated_ll_to_reduced_gg.linear-edition.test
    rotated_ll_to_rotated_ll.nn.test
    sh_to_healpix.intgrid-vod2uv.001.fail.test
    sh_to_healpix.intgrid-vod2uv.002.fail.test
    sh_to_healpix.intgrid.001.fail.test
    sh_to_healpix.intgrid.002.fail.test
    sh_to_lambert.regrid.test
    sh_to_lambert_azimuthal_equal_area.regrid.test
    sh_to_reduced_gg.intgrid-truncation.001.test
    sh_to_reduced_gg.intgrid-truncation.002.slow.test
    sh_to_reduced_gg.intgrid.slow.test
    sh_to_reduced_gg.octahedral-compatible.test
    sh_to_reduced_gg.octahedral.test
    sh_to_reduced_gg.reduced-compatible.test
    sh_to_reduced_gg.reduced.test
    sh_to_regular_ll.grib-packing-gridded-grib-edition-conversion.test
    sh_to_regular_ll.intgrid-av.test
    sh_to_regular_ll.intgrid-ccsds.001.test
    sh_to_regular_ll.intgrid-ccsds.002.test
    sh_to_regular_ll.intgrid-ccsds.003.test
    sh_to_regular_ll.intgrid-ccsds.004.test
    sh_to_regular_ll.intgrid-edition.001.test
    sh_to_regular_ll.intgrid-edition.002.test
    sh_to_regular_ll.intgrid-ieee.001.test
    sh_to_regular_ll.intgrid-ieee.002.test
    sh_to_regular_ll.intgrid-ieee.003.test
    sh_to_regular_ll.intgrid-ieee.004.test
    sh_to_regular_ll.intgrid-ieee.005.test
    sh_to_regular_ll.intgrid-ieee.006.test
    sh_to_regular_ll.intgrid-ieee.007.test
    sh_to_regular_ll.intgrid-simple.test
    sh_to_regular_ll.intgrid-so.001.test
    sh_to_regular_ll.intgrid-so.002.test
    sh_to_regular_ll.intgrid-so.003.test
    sh_to_regular_ll.intgrid.test
    sh_to_regular_ll.linear-uv2uv.001.test
    sh_to_regular_ll.linear-uv2uv.002.test
    sh_to_regular_ll.regrid.001.slow.test
    sh_to_regular_ll.regrid.004.test
    sh_to_regular_ll.scalar-gradient.test
    sh_to_regular_ll.truncation-uv2uv.001.test
    sh_to_regular_ll.truncation-uv2uv.002.test
    sh_to_regular_ll.vod2uv-intgrid.001.test
    sh_to_regular_ll.vod2uv-intgrid.002.test
    sh_to_regular_ll.vod2uv-intgrid.003.test
    sh_to_regular_ll.vod2uv-intgrid.004.test
    sh_to_regular_ll.vod2uv-intgrid.005.test
    sh_to_regular_ll.vod2uv-intgrid.006.test
    sh_to_regular_ll.vod2uv-intgrid.007.test
    sh_to_regular_ll.vod2uv-intgrid.008.test
    sh_to_regular_ll.vod2uv-uv-gradient.test
    sh_to_regular_ll.vod2uv.slow.test
    sh_to_rotated_ll.area-rotation.001.test
    sh_to_rotated_ll.area-rotation.002.test
    sh_to_rotated_ll.area-rotation.003.test
    sh_to_rotated_ll.truncation-rotation.test
    sh_to_rotated_ll.vod2uv-area.test
    sh_to_sh.bandpass.test
    sh_to_sh.cesaro-cesaro-k.test
    sh_to_sh.compatibility.001.test
    sh_to_sh.compatibility.002.test
    sh_to_sh.ieee.test
    sh_to_sh.simple.001.fail.test
    sh_to_sh.simple.002.fail.test
    sh_to_sh.simple.003.fail.test
    sh_to_sh.simple.004.fail.test
    sh_to_sh.truncation-simple.001.fail.test
    sh_to_sh.truncation-simple.002.fail.test
    sh_to_sh.truncation.001.test
    sh_to_sh.truncation.002.test
    sh_to_sh.truncation.003.test
    sh_to_sh.vod2uv-accuracy.test
    sh_to_sh.vod2uv-simple.fail.test
    sh_to_sh.vod2uv-truncation.001.test
    sh_to_sh.vod2uv-truncation.002.test
    sh_to_sh.vod2uv-truncation.003.test
    sh_to_sh.vod2uv-truncation.004.fail.test
    sh_to_sh.vod2uv.001.test
    sh_to_sh.vod2uv.002.test
    sh_to_sh.vod2uv.003.test
    sh_to_unstructured_grid.regrid.test
    sh_to_unstructured_grid.truncation-intgrid.test
    sh_to_unstructured_grid.vod2uv.test
    unstructured_grid_to_regular_ll.intermediate-interpolation-nn.test)

# ecCodes really doesn't build tools, so disabling testing is required
if(NOT eccodes_HAVE_BUILD_TOOLS)
    message(WARNING "${PROJECT_NAME}: regressions/ script tests require ecCodes option BUILD_TOOLS, tests disabled")
    return()
endif()

find_program(DIFF_TOOL NAMES "diff" "cmp" "false")
get_filename_component(MIR_TESTS_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../data" ABSOLUTE)
ecbuild_configure_file(mir-test.sh.in mir-test.sh @ONLY)

# how GRIB geometry is exchanged with ecCodes (see mir-test.sh.in), every test by each (legacy keeping the test name)
set(MODES legacy gridspec-input gridspec-output gridspec)

foreach(_t ${TESTS})
    if((_t MATCHES "[.]lsm[.]" AND NOT HAVE_MIR_DOWNLOAD_MASKS) OR
       (_t MATCHES "[.]proj[.]" AND NOT eckit_HAVE_PROJ))
        continue()
    endif()

    set(_environment ${_testEnvironment})
    if(_t MATCHES "[.]x$")
        list(APPEND _environment
            "MIR_CONFIG_AREA=${CMAKE_CURRENT_SOURCE_DIR}/config/area.yaml"
            "MIR_CONFIG_PARAMETERS=${CMAKE_CURRENT_SOURCE_DIR}/config/parameters.yaml")
    endif()

    set(_labels)
    if(_t MATCHES "[.]slow[.]")
        set(_labels slow)
    endif()

    target_from_path("${CMAKE_CURRENT_SOURCE_DIR}/${_t}" target)
    foreach(_mode ${MODES})
        set(_target ${target})
        if(NOT _mode STREQUAL "legacy")
            string(MAKE_C_IDENTIFIER "${target}_${_mode}" _target)
        endif()

        ecbuild_add_test(
            TARGET      ${_target}
            COMMAND     mir-test.sh
            ARGS        ${CMAKE_CURRENT_SOURCE_DIR}/${_t}
            LABELS      ${_labels} ${_mode}
            ENVIRONMENT ${_environment} MIR_TESTS_MODE=${_mode})

        if(_t MATCHES "[.]fail[.]")
            set_tests_properties(${_target} PROPERTIES WILL_FAIL TRUE)
        endif()
    endforeach()
endforeach()
