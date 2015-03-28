set( _mir_test_environment MIR_CACHE_DIR=${CMAKE_CURRENT_LIST_DIR}/cache )

ecbuild_add_test( TARGET      test_weight_cache
				  BOOST
				  SOURCES     test_weight_cache.cc 
				  ENVIRONMENT ${_mir_test_environment}
				  LIBS        mir eckit )
