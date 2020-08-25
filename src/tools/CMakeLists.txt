
# always-installed tools

ecbuild_add_executable(TARGET mir-tool OUTPUT_NAME mir SOURCES mir.cc LIBS mir)

unset(_tools)
list(APPEND _tools mir-count)

foreach(tool IN LISTS _tools)
    ecbuild_add_executable(TARGET ${tool} SOURCES ${tool}.cc LIBS mir)
endforeach()

# optionally-installed tools
unset(_tools)
list(APPEND _tools
    mir-climate-filter
    mir-compare  # NOTE: set in the testing scripts, however not used
    mir-config
    mir-formula
    mir-grid-info
    mir-inspect
    mir-list
    mir-load-legendre
    mir-load-matrix
    mir-make-lsm
    mir-points
    mir-statistics
    mir-weight-matrix-diff
    mir-gaussian-fractions
    mir-get-data
    mir-spectral-transform
    mir-spectral-transform-points
    mir-triangulate)

if (HAVE_MIR_TOOLS)
    foreach(tool IN LISTS _tools)
        ecbuild_add_executable(TARGET ${tool} SOURCES ${tool}.cc LIBS mir)
    endforeach()
else()
    foreach(tool IN LISTS _tools)
        ecbuild_add_executable(TARGET ${tool} SOURCES ${tool}.cc LIBS mir NOINSTALL)
    endforeach()
endif()
