
ecbuild_add_executable(TARGET mir-tool OUTPUT_NAME mir SOURCES mir.cc LIBS mir ${mir_INSTALL_TOOLS})

foreach(tool IN ITEMS
    mir-bounding-box
    mir-climate-filter
    mir-compare
    mir-compute
    mir-config-grib
    mir-config-param
    mir-count
    mir-formula
    mir-get-data
    mir-grid-box-area
    mir-grid-info
    mir-inspect
    mir-list
    mir-lsm
    mir-load-matrix
    mir-make-lsm
    mir-matrix-reorder
    mir-plot-lsm
    mir-points
    mir-statistics
    mir-version
    mir-weight-matrix-diff)
    ecbuild_add_executable(TARGET ${tool} SOURCES ${tool}.cc LIBS mir ${mir_INSTALL_TOOLS})
endforeach()

if(mir_HAVE_ECKIT_CODEC)
    foreach(tool IN ITEMS
        mir-codec-to-weight-matrix
        mir-grib-to-codec)
        ecbuild_add_executable(TARGET ${tool} SOURCES ${tool}.cc LIBS mir eckit_codec ${mir_INSTALL_TOOLS})
    endforeach()

    if(mir_HAVE_NETCDF)
        foreach(tool IN ITEMS
            mir-nc-to-codec
            mir-scrip-to-weight-matrix)
            ecbuild_add_executable(TARGET ${tool} SOURCES ${tool}.cc LIBS mir eckit_codec NetCDF::NetCDF_CXX ${mir_INSTALL_TOOLS})
        endforeach()
    endif()
endif()

if(mir_HAVE_ATLAS)
    foreach(tool IN ITEMS
        mir-load-legendre
        mir-spectral-transform-points
        mir-triangulate)
        ecbuild_add_executable(TARGET ${tool} SOURCES ${tool}.cc LIBS mir ${mir_INSTALL_TOOLS})
    endforeach()
endif()
