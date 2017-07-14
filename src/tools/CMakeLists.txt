
list(APPEND tools
    mir-compare
    mir-config
    mir-formula
    mir-inspect
    mir-list
    mir-make-lsm
    mir-shared-memory
    mir-statistics
    mir-tool
    mir-points
    mir-values-statistics
    mir-weight-matrix
)

if(HAVE_ATLAS)
list(APPEND tools
    mir-integrate
    mir-get-data
    mir-gaussian-fractions
)
endif()



foreach(tool ${tools})

    ecbuild_add_executable(
        TARGET      ${tool}
        SOURCES     ${tool}.cc
        INCLUDES    ${ATLAS_INCLUDE_DIRS}
        LIBS        mir )

endforeach()
