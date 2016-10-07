
foreach(tool mir-config mir-formula mir-inspect mir-make-lsm)
    ecbuild_add_executable(
        TARGET      ${tool}
        SOURCES     ${tool}.cc
        INCLUDES    ${ATLAS_INCLUDE_DIRS}
        LIBS        mir )
endforeach()

foreach(tool mir-compare mir-integrate mir-list mir-plot-lsm mir-shared-memory mir-statistics mir-tool)
    ecbuild_add_executable(
        TARGET      ${tool}
        SOURCES     ${tool}.cc
        INCLUDES    ${ATLAS_INCLUDE_DIRS}
        LIBS        mir eckit_option )
endforeach()
