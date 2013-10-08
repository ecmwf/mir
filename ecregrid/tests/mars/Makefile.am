
TESTS = spectral2grids.sh \
	reduced_gaussian2grids.sh \
	reduced_latlon2grids.sh

EXTRA_DIST = $(TESTS)
TESTS_ENVIRONMENT=MARS_HOME=@MARS_HOME@
## Make sure these will be cleaned even when they're not built by
## default.
CLEANFILES = *.grib data_out/*.grib
