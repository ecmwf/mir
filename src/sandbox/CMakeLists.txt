
ecbuild_add_executable(TARGET grids SOURCES grids.cc INCLUDES ${ATLAS_INCLUDE_DIRS} LIBS mir)
ecbuild_add_executable(TARGET lsm   SOURCES lsm.cc   INCLUDES ${ATLAS_INCLUDE_DIRS} LIBS mir)

