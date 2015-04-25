set( _mir_test_environment MIR_CACHE_DIR=${CMAKE_CURRENT_BINARY_DIR}/cache )


			  ecbuild_add_test(
				      TARGET    soyuz
				          SOURCES   soyuz.cc
					      RESOURCES test.bitmap test.grib test.nc sh.grib
					          INCLUDES  ${ECKIT_INCLUDE_DIRS} ${NETCDF_INCLUDE_DIRS}
						      LIBS      mir
						          )

