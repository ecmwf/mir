# (C) Copyright 1996- ECMWF.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
#
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation nor
# does it submit to any jurisdiction.


cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

find_package(ecbuild 3.4 REQUIRED HINTS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../ecbuild)

project(mir LANGUAGES CXX)

set(PERSISTENT_NAMESPACE "eckit" CACHE INTERNAL "") # needed for generating .b files for persistent support
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


ecbuild_add_option(FEATURE BUILD_TOOLS        DEFAULT OFF DESCRIPTION "build the command line tools (install)")
ecbuild_add_option(FEATURE MIR_DOWNLOAD_MASKS DEFAULT ON  DESCRIPTION "download LSM files")
ecbuild_add_option(FEATURE NETCDF             DEFAULT ON  DESCRIPTION "support for netCDF" REQUIRED_PACKAGES "NetCDF COMPONENTS C CXX")
ecbuild_add_option(FEATURE PNG                DEFAULT OFF DESCRIPTION "support for PNG encoding" REQUIRED_PACKAGES PNG)
ecbuild_add_option(FEATURE OMP                DEFAULT OFF DESCRIPTION "support for OpenMP shared memory parallelism" REQUIRED_PACKAGES "OpenMP COMPONENTS CXX")

if(NOT TARGET OpenMP::OpenMP_CXX)
    set(mir_HAVE_OMP 0)
endif()

ecbuild_find_package(NAME eccodes VERSION 2.40 REQUIRED)
ecbuild_find_package(NAME eckit   VERSION 1.29 REQUIRED)

ecbuild_add_option(FEATURE ATLAS DEFAULT ON DESCRIPTION "support for Atlas" REQUIRED_PACKAGES "atlas VERSION 0.36")

if(NOT TARGET eckit_geo)
    MESSAGE(FATAL_ERROR "eckit::geo (feature ECKIT_GEO) is required but not found")
endif()

set(MIR_LIBRARIES mir)

if(HAVE_BUILD_TOOLS)
    set(mir_INSTALL_TOOLS)
else()
    set(mir_INSTALL_TOOLS NOINSTALL)
endif()

include(cmake/compiler_warnings.cmake) # optionally handle compiler specific warnings

add_subdirectory(src)
add_subdirectory(etc)
add_subdirectory(share)

if(HAVE_TESTS)
    add_subdirectory(tests)
endif()


ecbuild_add_resources(TARGET ${PROJECT_NAME}_top_files SOURCES AUTHORS README.md NOTICE LICENSE INSTALL COPYING)
ecbuild_install_project(NAME ${PROJECT_NAME})

ecbuild_print_summary()
