file( GLOB config_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cfg" )

install( FILES ${config_files}
         DESTINATION ${INSTALL_DATA_DIR}/etc
         PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ )

foreach( _f ${config_files} )
  configure_file( ${_f} ${CMAKE_BINARY_DIR}/etc/${_f} COPYONLY )
endforeach()

### LSM's

# get them into the build dir

# ten-minutes.mask is a copy of emos-tables/lsm_32_lsm10m01

list( APPEND LSM_FILES  lsm.N640.grib lsm.N320.grib ten-minutes.mask 1km-lsm.mask)
foreach( _f ${LSM_FILES} )
  ecbuild_download_resource( ${CMAKE_BINARY_DIR}/etc/${_f} http://download.ecmwf.org/test-data/mir/lsm/${_f} )
endforeach()

# install them
foreach( _f ${LSM_FILES} )

  install( FILES ${CMAKE_BINARY_DIR}/etc/${_f}
           DESTINATION ${INSTALL_DATA_DIR}/etc
           PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ )

endforeach()

