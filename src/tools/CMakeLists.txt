
ecbuild_add_executable(
    TARGET      mir-tool
    SOURCES     mir-tool.cc
    INCLUDES    ${ATLAS_INCLUDE_DIRS}
    LIBS        mir eckit_option )

ecbuild_add_executable(
    TARGET      mir-formula
    SOURCES     mir-formula.cc
    INCLUDES    ${ATLAS_INCLUDE_DIRS}
    LIBS        mir )

ecbuild_add_executable(
    TARGET      mir-inspect
    SOURCES     mir-inspect.cc
    INCLUDES    ${ATLAS_INCLUDE_DIRS}
    LIBS        mir )

ecbuild_add_executable(
    TARGET      mir-compare
    SOURCES     mir-compare.cc
    INCLUDES    ${ATLAS_INCLUDE_DIRS}
    LIBS        mir eckit_option )

ecbuild_add_executable(
    TARGET      mir-config
    SOURCES     mir-config.cc
    INCLUDES    ${ATLAS_INCLUDE_DIRS}
    LIBS        mir )

ecbuild_add_executable(
    TARGET      mir-shared-memory
    SOURCES     mir-shared-memory.cc
    INCLUDES    ${ATLAS_INCLUDE_DIRS}
    LIBS        mir eckit_option )

ecbuild_add_executable(
    TARGET      mir-make-lsm
    SOURCES     mir-make-lsm.cc
    INCLUDES    ${ATLAS_INCLUDE_DIRS}
    LIBS        mir )

ecbuild_add_executable(
    TARGET      mir-plot-lsm
    SOURCES     mir-plot-lsm.cc
    INCLUDES    ${ATLAS_INCLUDE_DIRS}
    LIBS        mir eckit_option )

ecbuild_add_executable(
    TARGET      mir-stats
    SOURCES     mir-stats.cc
    INCLUDES    ${ATLAS_INCLUDE_DIRS}
    LIBS        mir eckit_option )

ecbuild_add_executable(
    TARGET      mir-integrate
    SOURCES     mir-integrate.cc
    INCLUDES    ${ATLAS_INCLUDE_DIRS}
    LIBS        mir eckit_option )

ecbuild_add_executable(
    TARGET      mir-list
    SOURCES     mir-list.cc
    INCLUDES    ${ATLAS_INCLUDE_DIRS}
    LIBS        mir eckit_option )

