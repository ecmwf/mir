
set(_dir "share/mir/masks")
foreach( _f
    "lsm.N640.grib"
    "lsm.N320.grib"
    "ten-minutes.mask"  # copy of libemos/tables/interpol/lsm_xx_lsm10m01
    "1km-lsm.mask" )

  # guarantee destination directory exists
  ecbuild_info("Downloading LSM: ${_dir}/${_f}")
  file( MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/${_dir}" )

  # get mask into the build dir
  ecbuild_download_resource(
    "${CMAKE_BINARY_DIR}/${_dir}/${_f}"
    "http://download.ecmwf.org/test-data/mir/${_dir}/${_f}" )

  # install mask
  install(
    FILES       "${CMAKE_BINARY_DIR}/${_dir}/${_f}"
    DESTINATION "${INSTALL_DATA_DIR}/${_dir}"
    PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ )

endforeach()

