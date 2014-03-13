ecbuild_add_test( TARGET      test_interpolate
				  SOURCES     test_interpolate.cc 
				  LIBS        mir eckit_grid eckit )

ecbuild_add_test( TARGET      test_weight_cache
				  SOURCES     test_weight_cache.cc 
				  LIBS        mir eckit )
