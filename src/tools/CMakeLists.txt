
ecbuild_add_executable(TARGET mir-tool OUTPUT_NAME mir SOURCES mir.cc LIBS mir)

unset(_tools)
list(APPEND _tools
    mir-compare
    mir-config
    mir-count
    mir-formula
    mir-inspect
    mir-list
    mir-make-lsm
    mir-points
    mir-shared-memory
    mir-statistics
    mir-values-statistics
    mir-weight-matrix
    mir-grid-info)

if(HAVE_ATLAS)
    list(APPEND _tools
        mir-gaussian-fractions
        mir-get-data
        mir-integrate
        mir-spectral-transform)
endif()

foreach(tool IN LISTS _tools)
    ecbuild_add_executable(TARGET ${tool} NOINSTALL SOURCES ${tool}.cc LIBS mir)
endforeach()

