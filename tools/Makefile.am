bin_PROGRAMS = ecregrid
ecregrid_SOURCES =  ecregrid.cpp \
		HandleRequest.h \
		HandleRequest.cpp

# set the include path
INCLUDES=  -I. -I$(top_builddir)/src $(GRIB_API_INC)

LDADD = $(top_builddir)/src/libecregrid.a  $(GRIB_API_LIB)
DEPENDENCIES = $(LIBADD)
