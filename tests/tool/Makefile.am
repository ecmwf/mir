
TESTS = regular_latlon2grids.sh \
	regular_latlon2grids_sfc.sh \
	spectral2spectral.sh \
	spectral2spectral_wind.sh \
	spectral2grids.sh \
	spectral2grids_wind.sh \
	reduced_gaussian2grids.sh \
	reduced_gaussian2grids_sfc.sh \
	regular_gaussian2grids.sh \
	regular_gaussian2grids_sfc.sh \
	reduced_latlon2grids.sh  \
	grid2grids_wind.sh  \
	pseudo_regular_gaussian2grids.sh \
	regular_latlon_cell_centered2grid.sh \
	multigribmessagesinfile.sh \
	subarea_extraction.sh \
	spectral2list_of_points.sh \
	sameAs.sh

EXTRA_DIST = include.sh $(TESTS) data_out/README

## temporary out of the list
##	average_weighted.sh \
##    frame_and_bitmap.sh
## Make sure these will be cleaned even when they're not built by
## default.
CLEANFILES = *.grib data_out/*.grib
