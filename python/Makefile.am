if WITH_PYTHON

noinst_SCRIPTS = setup.py

CLEANFILES = $(noinst_SCRIPTS) \
			ecregrid_swig.c \
			ecregrid_swig.py \
			_ecregrid_swig.so

# Extra stuff to go into the distribution
EXTRA_DIST = setup.py.in \
			ecregrid_swig.i \
			ecregrid_swig.py \
			ecregrid.py utils.h utils.c

# What I want installed
pdir = $(libdir)/python$(PYTHON_VERSION)/site-packages/ecregrid
p_DATA = \
	_ecregrid_swig.so \
	ecregrid.py 

do_subst = sed -e 's,[@]JASPER_ROOT[@],$(JASPER_ROOT),g' \
			-e 's,[@]GRIB_API_ROOT[@],$(GRIB_API_ROOT),g' \
			-e 's,[@]BUILD_DIR[@],$(top_builddir),g'

clean-local:
	-rm -rf build *.pyc *.so

setup.py: setup.py.in $(top_builddir)/src/libecregrid.a
	$(do_subst) < setup.py.in > setup.py

ecregrid_swig.py: ecregrid_swig.i
	p4 edit ecregrid_swig.py
	swig -python ecregrid_swig.i


ecregrid_swig.c: ecregrid_swig.i
	swig -python -module ecregrid_swig -o ecregrid_swig.c ecregrid_swig.i

_ecregrid_swig.so: setup.py ecregrid_swig.c ecregrid_swig.py 
	$(PYTHON) setup.py build_ext --inplace

include $(DEVEL_RULES)

endif
