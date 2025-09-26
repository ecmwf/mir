
if(HAVE_MIR_DOWNLOAD_MASKS)
    set(_dir "share/mir/masks")
    foreach(_f
        "lsm.10min.mask"
        "lsm.1km.climate.v013.mask"
        "lsm.1km.climate.v020.mask"
        "lsm.N128.grib"
        "lsm.N256.grib"
        "lsm.N320.grib"
        "lsm.N640.grib"
        "lsm.O1280.grib"
        "lsm.O320.grib"
        "lsm.O640.grib")

      # guarantee destination directory exists
      ecbuild_info("Downloading mask: ${_dir}/${_f}")
      file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/${_dir}")

      # get mask into the build dir
      ecbuild_download_resource(
        "${CMAKE_BINARY_DIR}/${_dir}/${_f}"
        "https://sites.ecmwf.int/repository/mir/${_dir}/${_f}")

      # install mask
      install(
        FILES       "${CMAKE_BINARY_DIR}/${_dir}/${_f}"
        DESTINATION "share/${PROJECT_NAME}/masks"
        PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ)

    endforeach()
endif()
