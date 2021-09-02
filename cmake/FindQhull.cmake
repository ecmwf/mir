# (C) Copyright 2011- ECMWF.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation nor
# does it submit to any jurisdiction.

# Find the Qhull library:
#   find_package(Qhull [REQUIRED])
#
# Variables checked:
#     Qhull_ROOT:         path containing include/ and lib/
#
# Variables set:
#     Qhull_FOUND:        if found
#     Qhull_LIBRARIES:    libraries paths
#     Qhull_INCLUDE_DIRS: include directory

# Use PkgConfig if possible
find_package(PkgConfig)

if(PKG_CONFIG_FOUND AND NOT Qhull_ROOT)
    pkg_check_modules(PKG_Qhull QUIET "qhullcpp")
else()
    if(NOT Qhull_ROOT)
        set(Qhull_ROOT "$ENV{Qhull_ROOT}")
    endif()
    set(Qhull_INCLUDE_DIRS "${Qhull_ROOT}/include")
    find_library(Qhull_LIBRARIES NAMES "qhull_r" PATHS ${Qhull_ROOT} PATH_SUFFIXES "lib" NO_DEFAULT_PATH)
endif()

message("Qhull_INCLUDE_DIRS: ${Qhull_INCLUDE_DIRS}")
message("Qhull_LIBRARIES: ${Qhull_LIBRARIES}")

# interface targets
set(_target Qhull::Qhull)
if(NOT TARGET ${_target})
    add_library(${_target} INTERFACE IMPORTED)
    target_link_libraries(${_target} INTERFACE ${Qhull_LIBRARIES})
    target_include_directories(${_target} INTERFACE ${Qhull_INCLUDE_DIRS})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Qhull REQUIRED_VARS Qhull_INCLUDE_DIRS Qhull_LIBRARIES)

mark_as_advanced(Qhull_INCLUDE_DIRS Qhull_LIBRARIES)

