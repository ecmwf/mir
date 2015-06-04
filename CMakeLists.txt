############################################################################################
#
#   mir - ECMWF Meterological Interpolation and Regriding
#
############################################################################################

cmake_minimum_required( VERSION 2.8.4 FATAL_ERROR )

project( mir C CXX )

set( CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../ecbuild/cmake")

include( ecbuild_system )

ecbuild_requires_macro_version( 1.8 )

###############################################################################
# project

ecbuild_use_package( PROJECT eckit  VERSION  0.8  REQUIRED )
ecbuild_use_package( PROJECT atlas  VERSION  0.3  REQUIRED )
# ecbuild_use_package( PROJECT libemos  )

# Experimental features in eckit are required
# TODO: remove requirement when no longer necessary
if( NOT ECKIT_HAVE_EXPERIMENTAL )
  message( FATAL_ERROR "Experimental features in eckit are required" )
endif()

# Either Eigen support or Armadillo support in eckit is required
if( NOT ECKIT_HAVE_EIGEN AND NOT ECKIT_HAVE_ARMADILLO )
  message( FATAL_ERROR "Either Eigen support or Armadillo support in eckit is required" )
endif()

# this will be replaced once ecbuild exports subcomponents between packages

ecbuild_declare_project()

###############################################################################
# some variables/options of this project

ecbuild_add_cxx11_flags()
ecbuild_check_cxx11( PRINT )

### OMP

ecbuild_add_option( FEATURE OMP
                    DESCRIPTION "support for OpenMP shared memory parallelism"
                    DEFAULT ON )

if( ENABLE_OMP )
  ecbuild_enable_omp()
else()
  ecbuild_enable_ompstubs()
endif()

### MPI

ecbuild_add_option( FEATURE MPI
                    DEFAULT OFF
                    DESCRIPTION "support for MPI distributed parallelism" )

if( ENABLE_MPI )

  ecbuild_enable_mpi()

  if( NOT MPI_CXX_FOUND )
    set( HAVE_MPI 0 )
  endif()

endif()

ecbuild_add_option( FEATURE NETCDF
                    DEFAULT ON
                    DESCRIPTION "support for NetCDF4"
                    REQUIRED_PACKAGES NetCDF4 )

### Hermes

ecbuild_add_option( FEATURE HERMES
                    DEFAULT OFF
                    DESCRIPTION "Hermes computation engine"
                    REQUIRED_PACKAGES "PROJECT hermes VERSION 0.1.0" )

###############################################################################
# contents

### export package to other ecbuild packages

set( MIR_INCLUDE_DIRS   ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_BINARY_DIR}/src )
set( MIR_LIBRARIES      mir )
set( MIR_TPLS           atlas eckit )

if( HAVE_HERMES )
  list( APPEND MIR_TPLS hermes )
endif()

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

add_subdirectory( src )
add_subdirectory( etc )
add_subdirectory( tests )

### installation

# install( DIRECTORY share DESTINATION ${CMAKE_INSTALL_PREFIX} )

ecbuild_add_resources( TARGET top_files SOURCES
                       TODO AUTHORS README NOTICE LICENSE INSTALL ChangeLog COPYING
                       DONT_PACK
                       BUGS )

############################################################################################
# packaging and installation

ecbuild_install_project( NAME ${PROJECT_NAME} )

ecbuild_print_summary()
