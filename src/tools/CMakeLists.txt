
ecbuild_add_executable(TARGET mir-tool OUTPUT_NAME mir SOURCES mir.cc LIBS mir ${mir_INSTALL_TOOLS})

foreach(tool IN ITEMS
    mir-climate-filter
    mir-compare  # NOTE: set in the testing scripts, however not used
    mir-compute
    mir-config
    mir-config-grib
    mir-count
    mir-formula
    mir-gaussian-fractions
    mir-get-data
    mir-grid-box-area
    mir-grid-info
    mir-inspect
    mir-list
    mir-load-matrix
    mir-make-lsm
    mir-matrix-reorder
    mir-plot-lsm
    mir-points
    mir-statistics
    mir-weight-matrix-diff)
    ecbuild_add_executable(TARGET ${tool} SOURCES ${tool}.cc LIBS mir ${mir_INSTALL_TOOLS})
endforeach()

if(mir_HAVE_ATLAS)
    foreach(tool IN ITEMS
        mir-load-legendre
        mir-spectral-transform-points
        mir-triangulate)
        ecbuild_add_executable(TARGET ${tool} SOURCES ${tool}.cc LIBS mir ${mir_INSTALL_TOOLS})
    endforeach()
endif()

if(mir_HAVE_ECTRANS)
    foreach(tool IN ITEMS
        mir-grid-to-spectral)
        ecbuild_add_executable(TARGET ${tool} SOURCES ${tool}.cc LIBS mir transi_dp ${mir_INSTALL_TOOLS})
    endforeach()
endif()

