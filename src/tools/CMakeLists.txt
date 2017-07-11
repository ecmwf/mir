
foreach(tool
    mir-compare
    mir-config
    mir-formula
    mir-gaussian-fractions
    mir-get-data
    mir-inspect
    mir-integrate
    mir-list
    mir-make-lsm
    mir-shared-memory
    mir-statistics
    mir-tool
    mir-values-statistics
    mir-weight-matrix )

    ecbuild_add_executable(
        TARGET      ${tool}
        SOURCES     ${tool}.cc
        INCLUDES    ${ATLAS_INCLUDE_DIRS}
        LIBS        mir )

endforeach()
