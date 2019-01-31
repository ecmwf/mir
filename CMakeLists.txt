cmake_minimum_required( VERSION 3.1 FATAL_ERROR )

project( mir C CXX )

set( CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../ecbuild/cmake")

include( ecbuild_system NO_POLICY_SCOPE )

ecbuild_requires_macro_version( 2.9 )

set( PERSISTENT_NAMESPACE "eckit" CACHE INTERNAL "" ) # needed for generating .b files for persistent support

ecbuild_declare_project()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

###############################################################################
# project

ecbuild_add_option( FEATURE MIR_TOOLS       DEFAULT OFF DESCRIPTION "install mir tools" )

ecbuild_add_option( FEATURE TEST_ASSERTIONS DEFAULT ON  DESCRIPTION "test: regressions" )
ecbuild_add_option( FEATURE OMP             DEFAULT ON  DESCRIPTION "support for OpenMP shared memory parallelism" )
ecbuild_add_option( FEATURE MPI             DEFAULT OFF DESCRIPTION "support for MPI distributed parallelism" )
ecbuild_add_option( FEATURE NETCDF          DEFAULT ON  DESCRIPTION "support for NetCDF4" REQUIRED_PACKAGES NetCDF4 )


ecbuild_use_package( PROJECT eccodes VERSION 2.4.1 REQUIRED )
ecbuild_use_package( PROJECT eckit   VERSION 0.18.0 REQUIRED )
ecbuild_use_package( PROJECT atlas   VERSION 0.11)
#ecbuild_use_package( PROJECT metkit  VERSION  0.7.0 )

if(ECCODES_FOUND)
    set(HAVE_ECCODES 1)
endif()

if(ATLAS_FOUND)
    set(HAVE_ATLAS 1)
endif()


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
