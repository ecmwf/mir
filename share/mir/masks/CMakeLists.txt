
# get them into the build dir
ecbuild_info("Downloading LSM files")

list( APPEND LSM_FILES
    "lsm.N640.grib"
    "lsm.N320.grib"
    "ten-minutes.mask"  # copy of libemos/tables/interpol/lsm_32_lsm10m01
    "1km-lsm.mask" )

file( MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/share/mir/masks" )
foreach( _f ${LSM_FILES} )
  ecbuild_download_resource(
      "${CMAKE_BINARY_DIR}/share/mir/masks/${_f}"
      "http://download.ecmwf.org/test-data/mir/lsm/${_f}" )
endforeach()

# install them
foreach( _f ${LSM_FILES} )
  install(
      FILES       "${CMAKE_BINARY_DIR}/share/mir/masks/${_f}"
      DESTINATION "${INSTALL_DATA_DIR}/share/mir/masks"
      PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ )
endforeach()

