cmake_minimum_required(VERSION 3.6 FATAL_ERROR)
if(POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW)
endif()

find_package(ecbuild 3.1 REQUIRED)

project(mir VERSION 1.5.2 LANGUAGES CXX)

set(PERSISTENT_NAMESPACE "eckit" CACHE INTERNAL "") # needed for generating .b files for persistent support
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

###############################################################################
# project

ecbuild_add_option( FEATURE MIR_TOOLS          DEFAULT OFF DESCRIPTION "install mir tools" )
ecbuild_add_option( FEATURE MIR_DOWNLOAD_MASKS DEFAULT ON  DESCRIPTION "download LSM files" )

ecbuild_add_option( FEATURE TEST_ASSERTIONS    DEFAULT ON  DESCRIPTION "test: regressions" )
ecbuild_add_option( FEATURE OMP                DEFAULT ON  DESCRIPTION "support for OpenMP shared memory parallelism" )
ecbuild_add_option( FEATURE MPI                DEFAULT OFF DESCRIPTION "support for MPI distributed parallelism" )
ecbuild_add_option( FEATURE NETCDF             DEFAULT ON  DESCRIPTION "support for NetCDF" REQUIRED_PACKAGES "NetCDF COMPONENTS C" )
ecbuild_add_option( FEATURE PNG                DEFAULT OFF DESCRIPTION "support for PNG encoding" REQUIRED_PACKAGES PNG )

ecbuild_use_package( PROJECT eccodes VERSION 2.17 REQUIRED )
ecbuild_use_package( PROJECT eckit   VERSION 1.10 REQUIRED )
ecbuild_use_package( PROJECT atlas   VERSION 0.20 REQUIRED )

if(ATLAS_FOUND)
    set(HAVE_ATLAS 1)
endif()

if(HAVE_PNG)
    add_definitions(${PNG_DEFINITIONS})
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
    if( DEFINED ${TPL}_DEFINITIONS )
      list( APPEND MIR_EXTRA_DEFINITIONS ${${TPL}_DEFINITIONS} )
    endif()
    if( DEFINED ${TPL}_TPL_DEFINITIONS )
      list( APPEND MIR_EXTRA_DEFINITIONS ${${TPL}_TPL_DEFINITIONS} )
    endif()

    if( DEFINED ${TPL}_INCLUDE_DIRS )
      list( APPEND MIR_EXTRA_INCLUDE_DIRS ${${TPL}_INCLUDE_DIRS} )
    endif()
    if( DEFINED ${TPL}_TPL_INCLUDE_DIRS )
      list( APPEND MIR_EXTRA_INCLUDE_DIRS ${${TPL}_TPL_INCLUDE_DIRS} )
    endif()

    if( DEFINED ${TPL}_LIBRARIES )
      list( APPEND MIR_EXTRA_LIBRARIES ${${TPL}_LIBRARIES} )
    endif()
    if( DEFINED ${TPL}_TPL_LIBRARIES )
      list( APPEND MIR_EXTRA_LIBRARIES ${${TPL}_TPL_LIBRARIES} )
    endif()
  endif()
endforeach()

include_directories( ${MIR_INCLUDE_DIRS} )
if( DEFINED MIR_EXTRA_INCLUDE_DIRS )
  include_directories( ${MIR_EXTRA_INCLUDE_DIRS} )
endif()

add_subdirectory( "src" )
add_subdirectory( "etc" )
add_subdirectory( "share" )
add_subdirectory( "tests" )

## print all variables
#get_cmake_property(_variableNames VARIABLES)
#foreach(_variableName ${_variableNames})
#  ecbuild_info("${_variableName}=${${_variableName}}")
#endforeach()


###############################################################################
# installation

# install( DIRECTORY share DESTINATION ${CMAKE_INSTALL_PREFIX} )

ecbuild_add_resources(TARGET ${PROJECT_NAME}_top_files
                      SOURCES AUTHORS README NOTICE LICENSE INSTALL COPYING
                      DONT_PACK BUGS TODO)

ecbuild_install_project( NAME ${PROJECT_NAME} )

ecbuild_print_summary()

