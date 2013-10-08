
check_PROGRAMS = prodgen
#TESTS = $(check_PROGRAMS)

prodgen_SOURCES = prodgen.cpp

# set the include path
INCLUDES = -I$(top_builddir)/src -I$(top_builddir)/src/api

LDADD = $(top_builddir)/src/libecregrid.a $(CXX_LIB)
