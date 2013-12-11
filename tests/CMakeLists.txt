ecbuild_add_test(   TARGET      test_interpolate
		    SOURCES     test_interpolate.cc
		    CONDITION	ECKIT_GRID
		    LIBS        eckit eckit_grid mir )

