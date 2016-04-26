############################################################################################
#
#   MIR - ECMWF Meterological Interpolation and Regriding
#
############################################################################################

cmake_minimum_required( VERSION 2.8.4 FATAL_ERROR )

project( mir C CXX )

set( CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../ecbuild/cmake")

include( ecbuild_system NO_POLICY_SCOPE )

ecbuild_requires_macro_version( 1.9 )

###############################################################################
# project

ecbuild_use_package( PROJECT grib_api VERSION  1.14 REQUIRED )
ecbuild_use_package( PROJECT eckit  VERSION  0.9  REQUIRED )
ecbuild_use_package( PROJECT atlas  VERSION  0.6  REQUIRED )

# this will be replaced once ecbuild exports subcomponents between packages

ecbuild_declare_project()

###############################################################################
# some variables/options of this project

ecbuild_add_cxx11_flags()
ecbuild_check_cxx11( PRINT )

ecbuild_add_option( FEATURE OMP               DEFAULT ON  DESCRIPTION "support for OpenMP shared memory parallelism" )
ecbuild_add_option( FEATURE MPI               DEFAULT OFF DESCRIPTION "support for MPI distributed parallelism" )
ecbuild_add_option( FEATURE NETCDF            DEFAULT ON  DESCRIPTION "support for NetCDF4" REQUIRED_PACKAGES NetCDF4 )
ecbuild_add_option( FEATURE MIR_TESTS_REGRESS DEFAULT OFF DESCRIPTION "additional tests: regression" )

# OMP/MPI features handling
if( ENABLE_OMP )
  ecbuild_enable_omp()
else()
  ecbuild_enable_ompstubs()
endif()

if( ENABLE_MPI )
  ecbuild_enable_mpi()
  if( NOT MPI_CXX_FOUND )
    set( HAVE_MPI 0 )
  endif()
endif()

if( HAVE_NETCDF )
  list( APPEND MIR_FEATURE_LIBRARIES ${NETCDF_LIBRARIES} )
endif()

###############################################################################
# contents

### export package to other ecbuild packages

set( MIR_INCLUDE_DIRS   ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_BINARY_DIR}/src )
set( MIR_LIBRARIES      mir )

get_directory_property( COMPILE_DEFINITIONS MIR_DEFINITIONS )

foreach( _tpl ${MIR_TPLS} )
	string( TOUPPER ${_tpl} TPL )
	if( ${TPL}_FOUND )
		list( APPEND MIR_EXTRA_DEFINITIONS   ${${TPL}_DEFINITIONS} ${${TPL}_TPL_DEFINITIONS})
		list( APPEND MIR_EXTRA_INCLUDE_DIRS  ${${TPL}_INCLUDE_DIRS} ${${TPL}_TPL_INCLUDE_DIRS} )
		list( APPEND MIR_EXTRA_LIBRARIES     ${${TPL}_LIBRARIES} ${${TPL}_TPL_LIBRARIES})
	endif()
endforeach()

### include directories

include_directories( ${MIR_INCLUDE_DIRS} ${MIR_EXTRA_INCLUDE_DIRS}  )

add_subdirectory( "src" )
add_subdirectory( "etc" )
add_subdirectory( "share" )
add_subdirectory( "tests" )

### installation

# install( DIRECTORY share DESTINATION ${CMAKE_INSTALL_PREFIX} )

ecbuild_add_resources( TARGET ${PROJECT_NAME}_top_files SOURCES
                       TODO AUTHORS README NOTICE LICENSE INSTALL ChangeLog COPYING
                       DONT_PACK
                       BUGS )

############################################################################################
# packaging and installation

ecbuild_install_project( NAME ${PROJECT_NAME} )

ecbuild_print_summary()
