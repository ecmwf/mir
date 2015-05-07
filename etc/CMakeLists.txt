file( GLOB config_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cfg" )

install( FILES ${config_files}
         DESTINATION share/${PROJECT_NAME}/etc
		 PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ )

### LSM's

# get them into the build dir

list( APPEND LSM_FILES  lsm.N640.grib lsm.N320.grib )
foreach( _f ${LSM_FILES} )
	ecbuild_download_resource( ${CMAKE_CURRENT_SOURCE_DIR}/${_f} http://download.ecmwf.org/test-data/mir/lsm/${_f} )
endforeach()

# install them
foreach( _f ${LSM_FILES} )

install( FILES ${CMAKE_CURRENT_SOURCE_DIR}/${_f}
         DESTINATION share/${PROJECT_NAME}/etc
		 PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ )

endforeach()

