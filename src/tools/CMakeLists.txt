
list(APPEND tools
    mir-compare
    mir-config
    mir-count
    mir-formula
    mir-info
    mir-inspect
    mir-list
    mir-make-lsm
    mir-points
    mir-shared-memory
    mir-statistics
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


ecbuild_add_executable(
    TARGET      mir-tool
    OUTPUT_NAME mir
    SOURCES     mir.cc
    INCLUDES    ${ATLAS_INCLUDE_DIRS}
    LIBS        mir )


foreach(tool ${tools})
    ecbuild_add_executable(
        TARGET      ${tool}
        SOURCES     ${tool}.cc
        INCLUDES    ${ATLAS_INCLUDE_DIRS}
        LIBS        mir )
endforeach()

