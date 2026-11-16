# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

# a test per family of actions (by action type), performed on synthetic fields, and of action specs, contexts,
# end actions (io), graphs, plans and styles (preparing plans)
set(TESTS
    action_graph
    actionspec
    area
    context
    filter
    filter.sh
    filter.winds-nabla
    interpolate.gg
    interpolate.grid
    interpolate.ll
    io
    misc
    plan
    style
    transform.gg
    transform.grid
    transform.ll)

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
