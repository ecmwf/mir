
# Setup cache directory
set(TEST_ECKIT_CACHE_DIR ${CMAKE_CURRENT_BINARY_DIR}/cache/)
file(REMOVE_RECURSE ${TEST_ECKIT_CACHE_DIR})

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

