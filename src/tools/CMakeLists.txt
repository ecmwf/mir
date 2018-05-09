
ecbuild_add_executable(TARGET mir-tool OUTPUT_NAME mir SOURCES mir.cc LIBS mir)


foreach(tool
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
    mir-grid-info)
    ecbuild_add_executable(TARGET ${tool} SOURCES ${tool}.cc LIBS mir)
endforeach()


if(HAVE_ATLAS)
    foreach(tool
        mir-gaussian-fractions
        mir-get-data
        mir-integrate
        mir-spectral-transform)
        ecbuild_add_executable(TARGET ${tool} SOURCES ${tool}.cc LIBS mir)
    endforeach()
endif()

