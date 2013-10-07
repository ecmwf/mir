
noinst_PROGRAMS = ecregrid_mars ecregrid_mars_wind ecregrid_dissemination ecregrid_unpacked_to_grib ecregrid_dissemination_mix ecregrid_list_to_grib ecregrid_to_one_point ecregrid_to_list_of_points ecregrid_to_one_point_loop

ecregrid_mars_SOURCES = ecregrid_mars.c
ecregrid_mars_wind_SOURCES = ecregrid_mars_wind.c
ecregrid_dissemination_SOURCES = ecregrid_dissemination.c
ecregrid_dissemination_mix_SOURCES = ecregrid_dissemination_mix.c
ecregrid_unpacked_to_grib_SOURCES = ecregrid_unpacked_to_grib.c
ecregrid_list_to_grib_SOURCES = ecregrid_list_to_grib.c
ecregrid_to_one_point_SOURCES = ecregrid_to_one_point.c
ecregrid_to_list_of_points_SOURCES = ecregrid_to_list_of_points.c
ecregrid_to_one_point_loop_SOURCES = ecregrid_to_one_point_loop.c

# set the include path
INCLUDES=  -I. -I$(top_builddir)/src $(GRIB_API_INC)

LDADD = $(top_builddir)/src/libecregrid.a $(GRIB_API_LIB) $(CXX_LIB)
DEPENDENCIES = $(LIBADD)

CLEANFILES = ecregrid_mars ecregrid_mars_wind ecregrid_dissemination ecregrid_unpacked_to_grib ecregrid_dissemination_mix ecregrid_to_one_point ecregrid_to_list_of_points ecregrid_to_one_point_loop

