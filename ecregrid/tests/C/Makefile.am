
check_PROGRAMS = ecregrid_mars ecregrid_mars_wind ecregrid_dissemination ecregrid_unpacked_to_grib ecregrid_dissemination_mix
#TESTS = $(check_PROGRAMS)

ecregrid_mars_SOURCES = ecregrid_mars.c
ecregrid_mars_wind_SOURCES = ecregrid_mars_wind.c
ecregrid_dissemination_SOURCES = ecregrid_dissemination.c
ecregrid_dissemination_mix_SOURCES = ecregrid_dissemination_mix.c
ecregrid_unpacked_to_grib_SOURCES = ecregrid_unpacked_to_grib.c

# set the include path
INCLUDES = -I$(top_builddir)/src $(GRIB_API_INC)

LDADD = $(top_builddir)/src/libecregrid.a $(GRIB_API_LIB) $(CXX_LIB)
