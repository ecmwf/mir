
ecbuild_add_executable(TARGET lsm SOURCES lsm.cc LIBS mir ${mir_INSTALL_TOOLS})

if(mir_HAVE_ATLAS)
    ecbuild_add_executable(TARGET grids SOURCES grids.cc LIBS mir ${mir_INSTALL_TOOLS})
endif()
