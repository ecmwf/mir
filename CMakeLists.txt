############################################################################################
# cmake options:
#
#       -DCMAKE_BUILD_TYPE=Debug|RelWithDebInfo|Release|Production
#       -DCMAKE_INSTALL_PREFIX=/path/to/install
#
#       -DCMAKE_MODULE_PATH=/path/to/ecbuild/cmake
#
#       -DCMAKE_C_COMPILER=gcc
#       -DCMAKE_C_COMPILER=g++
#
#       -DCMAKE_PREFIX_PATH=/path/to/jasper:/path/to/any/package/out/of/place
#
#       -DGRIB_API_PATH=/path/to/grib

cmake_minimum_required( VERSION 2.8.4 FATAL_ERROR )

project( mir C CXX )

set( CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH} )

include( ecbuild_system )

if( ECBUILD_MACRO_VERSION VERSION_LESS 1.1 )
    message( FATAL_ERROR "${PROJECT_NAME} needs ecbuild >= 1.1" )
endif()

###############################################################################
# local project

ecbuild_use_package( PROJECT eckit    VERSION  0.4.0 REQUIRED )
ecbuild_use_package( PROJECT grib_api VERSION 1.10.0 REQUIRED )

ecbuild_declare_project()

###############################################################################
# setting paths

set( MIR_SHARE_DIR "${CMAKE_INSTALL_PREFIX}/share/mir"     ${MIR_SHARE_PATH} )
set( MIR_DATA_DIR  "${CMAKE_INSTALL_PREFIX}/share/mir/lsm" ${MIR_LSM_PATH} )

###############################################################################
# some variables/options of this project

option( MIR_PYTHON "whether or not to build the MIR python interface" OFF )

###############################################################################
# find external dependencies

find_package( Threads )

find_package( SWIG )
if( SWIG_FOUND )
    include( ${SWIG_USE_FILE} )
endif()

find_package( Eigen3 3.2 )

###############################################################################
# contents

# for package export to other ecbuild packages
set( MIR_INCLUDE_DIRS   ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_BINARY_DIR}/src )
set( MIR_LIBRARIES      ${CMAKE_PROJECT_NAME} )
set( MIR_TPLS           grib_api )

include_directories( ${MIR_INCLUDE_DIRS}  )
include_directories( ${GRIB_API_INCLUDE_DIRS} )

### config header

set_directory_properties( PROPERTIES COMPILE_DEFINITIONS "${GRIB_API_DEFINITIONS}" )

add_subdirectory( ecregrid )

install( DIRECTORY share DESTINATION ${CMAKE_INSTALL_PREFIX} )

ecbuild_add_resources( TARGET top_files SOURCES
											 TODO AUTHORS README NOTICE LICENSE INSTALL ChangeLog COPYING
										DONT_PACK
											 BUGS )

############################################################################################
# packaging and installation

ecbuild_install_project( NAME ${CMAKE_PROJECT_NAME} )

ecbuild_print_summary()
