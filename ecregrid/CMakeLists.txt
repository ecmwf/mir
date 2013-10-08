set( ECREGRID_MAJOR_VERSION    1 )
set( ECREGRID_MINOR_VERSION    9 )
set( ECREGRID_PATCH_VERSION 0 )

set( ECREGRID_VERSION_STR "${ECREGRID_MAJOR_VERSION}.${ECREGRID_MINOR_VERSION}.${ECREGRID_PATCH_VERSION}" )

set( ECREGRID_SHARE_DIR "${CMAKE_INSTALL_PREFIX}/share/ecregrid" )
set( ECREGRID_DATA_DIR  "${CMAKE_INSTALL_PREFIX}/share/ecregrid/lsm" )

######################################################################

configure_file( src/cmake_config.h.in src/ecregrid_config.h )

set( ECREGRID_INCLUDE_DIRS   ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_BINARY_DIR}/src )

include_directories( ${ECREGRID_INCLUDE_DIRS}  )

add_subdirectory( src )
add_subdirectory( tools )
add_subdirectory( tests )
