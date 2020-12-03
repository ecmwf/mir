
if(HAVE_BUILD_TOOLS)
    ecbuild_add_executable(TARGET mir-tool OUTPUT_NAME mir SOURCES mir.cc LIBS mir)
else()
    ecbuild_add_executable(TARGET mir-tool OUTPUT_NAME mir SOURCES mir.cc LIBS mir NOINSTALL)
endif()

foreach(tool IN ITEMS
        mir-climate-filter
        mir-compare  # NOTE: set in the testing scripts, however not used
        mir-config
        mir-count
        mir-formula
        mir-gaussian-fractions
        mir-get-data
        mir-grid-info
        mir-inspect
        mir-list
        mir-load-legendre
        mir-load-matrix
        mir-make-lsm
        mir-points
        mir-spectral-transform
        mir-spectral-transform-points
        mir-statistics
        mir-triangulate
        mir-weight-matrix-diff)
    if(HAVE_BUILD_TOOLS)
        ecbuild_add_executable(TARGET ${tool} SOURCES ${tool}.cc LIBS mir)
    else()
        ecbuild_add_executable(TARGET ${tool} SOURCES ${tool}.cc LIBS mir NOINSTALL)
    endif()
endforeach()

