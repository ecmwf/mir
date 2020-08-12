if(HAVE_ATLAS)
    ecbuild_add_executable(TARGET grids SOURCES grids.cc LIBS mir)
endif()

ecbuild_add_executable(TARGET lsm SOURCES lsm.cc INCLUDES LIBS mir)

