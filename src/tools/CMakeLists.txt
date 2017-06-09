
foreach(tool mir-config mir-formula mir-inspect mir-make-lsm)
    ecbuild_add_executable(
        TARGET      ${tool}
        SOURCES     ${tool}.cc
        INCLUDES    ${ATLAS_INCLUDE_DIRS}
        LIBS        mir )
endforeach()

# mir-plot-lsm

foreach(tool mir-compare mir-get-data mir-integrate mir-list mir-shared-memory mir-statistics mir-tool mir-weight-matrix)
    ecbuild_add_executable(
        TARGET      ${tool}
        SOURCES     ${tool}.cc
        INCLUDES    ${ATLAS_INCLUDE_DIRS}
        LIBS        mir eckit_option )
endforeach()
