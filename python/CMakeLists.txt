cmake_minimum_required( VERSION 2.6.4 FATAL_ERROR )

if (ECREGRID_PYTHON_ON)
	
list( APPEND CLEANFILES
         setup.py
	 ecregrid_swig.c
	 ecregrid_swig.py
	 _ecregrid_swig.so
    )

list( APPEND EXTRA_DIST
         setup.py.in
	 ecregrid_swig.i
	 ecregrid_swig.py
	 ecregrid.py
	 utils.h
	 utils.c
    )

set (DO_SUBST "sed -e 's,[@]JASPER_ROOT[@],$(JASPER_ROOT),g' \
                   -e 's,[@]GRIB_API_ROOT[@],$(GRIB_API_ROOT),g' \
                   -e 's,[@]BUILD_DIR[@],$(top_builddir),g'")

add_custom_target( setup.py
	COMMAND ${DO_SUBST} < setup.py.in > setup.py
        DEPENDS setup.py.in
) 

endif (ECREGRID_PYTHON_ON)
