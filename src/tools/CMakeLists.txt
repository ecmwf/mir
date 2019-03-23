
# NOTE: mir-compare is also set in the testing scripts (however not used)
ecbuild_add_executable(TARGET mir-tool OUTPUT_NAME mir SOURCES mir.cc LIBS mir)

unset(_tools)
list(APPEND _tools
    mir-compare
    mir-config
    mir-count
    mir-formula
    mir-grid-info
    mir-inspect
    mir-list
    mir-make-lsm
    mir-points
    mir-shared-memory
    mir-statistics
    mir-weight-matrix)

if(HAVE_ATLAS)
    list(APPEND _tools
        mir-gaussian-fractions
        mir-get-data
        mir-spectral-transform)
endif()

if (HAVE_MIR_TOOLS)
    foreach(tool IN LISTS _tools)
        ecbuild_add_executable(TARGET ${tool} SOURCES ${tool}.cc LIBS mir)
    endforeach()
else()
    foreach(tool IN LISTS _tools)
        ecbuild_add_executable(TARGET ${tool} SOURCES ${tool}.cc LIBS mir NOINSTALL)
    endforeach()
endif()

