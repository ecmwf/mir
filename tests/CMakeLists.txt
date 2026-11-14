# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0


# Setup cache directory
set(MIR_CACHE_PATH "${CMAKE_CURRENT_BINARY_DIR}/cache")
file(COPY cache DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

# Set testing environment
set(_testEnvironment
    "MIR_DEBUG=1"
    "ECCODES_ECKIT_GEO=1"
    "MIR_HOME=${CMAKE_BINARY_DIR}"
    "MIR_CACHE_PATH=${MIR_CACHE_PATH}"
    "TEST_ECKIT_CACHE_DIR=${MIR_CACHE_PATH}")

if(NOT eccodes_HAVE_MEMFS)
    if(ECCODES_DEFINITION_PATH)
        list(APPEND _testEnvironment "ECCODES_DEFINITION_PATH=${ECCODES_DEFINITION_PATH}")
    endif()
    if(ECCODES_SAMPLES_PATH)
        list(APPEND _testEnvironment "ECCODES_SAMPLES_PATH=${ECCODES_SAMPLES_PATH}")
    endif()
endif()

function(target_from_path path out_var)
    file(RELATIVE_PATH _rel "${CMAKE_SOURCE_DIR}" "${path}")
    string(MAKE_C_IDENTIFIER "${_rel}" _id)
    if(NOT _id MATCHES "^mir_")
        set(_id "mir_${_id}")
    endif()
    string(REGEX REPLACE "_test$" "" result "${_id}")
    if(NOT _id MATCHES "^mir_")
        set(_id "mir_${_id}")
    endif()
    set(${out_var} "${result}" PARENT_SCOPE)
endfunction()

if(eccodes_HAVE_BUILD_TOOLS)
    add_subdirectory(tool)
else()
    message(WARNING "${PROJECT_NAME}: tool/ tests require ecCodes option BUILD_TOOLS, tests disabled")
endif()


add_subdirectory(regressions)
add_subdirectory(unit)
