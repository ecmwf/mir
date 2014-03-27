############################################################################################
#
#   mir - ECMWF Meterological Interpolation and Regriding
#
############################################################################################
### TODO
#
#   * cache weights based also on algorithm
#   * define a MIR_HOME and resource file to store configurations
#   * define a MIR_DATA_CACHE
#   * cache mesh generation
#   * when in and out grid hashes match we should immedietly return identity matrix

cmake_minimum_required( VERSION 2.8.4 FATAL_ERROR )

project( mir C CXX )

set( CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../ecbuild/cmake")

include( ecbuild_system )

ecbuild_requires_macro_version( 1.3 )

###############################################################################
# local project

ecbuild_use_package( PROJECT eckit  VERSION  0.6  REQUIRED )
ecbuild_use_package( PROJECT atlas  VERSION  0.1  REQUIRED )

ecbuild_declare_project()

###############################################################################
# some variables/options of this project

set( MIR_SHARE_DIR "${CMAKE_INSTALL_PREFIX}/share/mir"     ${MIR_SHARE_PATH} )
set( MIR_DATA_DIR  "${CMAKE_INSTALL_PREFIX}/share/mir/lsm" ${MIR_LSM_PATH} )

ecbuild_add_cxx11_flags()
ecbuild_check_cxx11( PRINT )

###############################################################################
# contents

### export package to other ecbuild packages

set( MIR_INCLUDE_DIRS   ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_BINARY_DIR}/src )
set( MIR_LIBRARIES      ${CMAKE_PROJECT_NAME} )
set( MIR_TPLS           atlas eckit )

get_directory_property( COMPILE_DEFINITIONS MIR_DEFINITIONS )

foreach( _tpl ${MIR_TPLS} )
	string( TOUPPER ${_tpl} TPL )
	if( ${TPL}_FOUND )
		list( APPEND MIR_EXTRA_DEFINITIONS   ${${TPL}_DEFINITIONS} )
		list( APPEND MIR_EXTRA_INCLUDE_DIRS  ${${TPL}_INCLUDE_DIRS} )
		list( APPEND MIR_EXTRA_LIBRARIES     ${${TPL}_LIBRARIES} )
	endif()
endforeach()

### include directories

include_directories( ${MIR_INCLUDE_DIRS} ${MIR_EXTRA_INCLUDE_DIRS}  )

add_subdirectory( src )

### installation

# install( DIRECTORY share DESTINATION ${CMAKE_INSTALL_PREFIX} )

ecbuild_add_resources( TARGET top_files SOURCES
											 TODO AUTHORS README NOTICE LICENSE INSTALL ChangeLog COPYING
										DONT_PACK
											 BUGS )

############################################################################################
# packaging and installation

ecbuild_install_project( NAME ${CMAKE_PROJECT_NAME} )

ecbuild_print_summary()
