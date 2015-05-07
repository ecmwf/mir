file( GLOB config_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cfg" )

install( FILES ${config_files} 
         DESTINATION share/${PROJECT_NAME}/etc
		 PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ )

### LSM's

# get them into the build dir

list( APPEND LSM_FILES  lsm.N640.grib lsm.N320.grib )
foreach( _f ${LSM_FILES} ) 
	ecbuild_download_resource( ${_f} http://download.ecmwf.org/test-data/mir/lsm/${_f} )
endforeach()

# install them

install( FILES ${LSM_FILES}
         DESTINATION share/${PROJECT_NAME}/etc
		 PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ )


