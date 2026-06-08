# (C) Copyright 1996- ECMWF.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation nor
# does it submit to any jurisdiction.

# NumPy was listed in REQUIRED_PACKAGES; re-invoke to populate Python3_NumPy_INCLUDE_DIRS
find_package(Python3 REQUIRED COMPONENTS NumPy)

find_program(CYTHON_EXECUTABLE NAMES cython cython3
    HINTS ${Python3_ROOT_DIR}/bin
    DOC "Cython transpiler"
    REQUIRED)

message(STATUS "mir Python bindings: Cython=${CYTHON_EXECUTABLE}")

set(_src ${CMAKE_CURRENT_SOURCE_DIR}/mir/src/_mir)

set(_cython_flags
    --cplus
    --directive language_level=3
    --directive c_string_type=unicode
    --directive c_string_encoding=utf8
)

# Install destination: prefix-relative site-packages
execute_process(
    COMMAND ${Python3_EXECUTABLE} -c "import sysconfig; print(sysconfig.get_path('platlib', vars={'platbase': '${CMAKE_INSTALL_PREFIX}', 'base': '${CMAKE_INSTALL_PREFIX}'}))"
    OUTPUT_VARIABLE _python_sitearch
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(MIR_PYTHON_INSTALL_DIR ${_python_sitearch} CACHE PATH
    "Install directory for mir Python extensions")

set(_mir_cpp ${CMAKE_CURRENT_BINARY_DIR}/_mir.cpp)
add_custom_command(
    OUTPUT ${_mir_cpp}
    COMMAND ${CYTHON_EXECUTABLE} ${_cython_flags}
        -I ${_src}
        ${_src}/_mir.pyx
        -o ${_mir_cpp}
    DEPENDS
        ${_src}/_mir.pyx
        ${_src}/mir_defs.pxd
        ${_src}/eckit_defs.pxd
        ${_src}/std_defs.pxd
    COMMENT "Cythonizing _mir.pyx"
    VERBATIM
)

Python3_add_library(_mir MODULE WITH_SOABI
    ${_mir_cpp}
    ${_src}/mir/input/ArrayInput.cc
    ${_src}/mir/input/PyGribInput.cc
    ${_src}/mir/output/PyGribOutput.cc
)

target_include_directories(_mir PRIVATE ${PROJECT_SOURCE_DIR}/src ${PROJECT_BINARY_DIR}/src ${_src} ${Python3_NumPy_INCLUDE_DIRS})
target_link_libraries(_mir PRIVATE mir)
target_compile_definitions(_mir PRIVATE NPY_NO_DEPRECATED_API=NPY_1_7_API_VERSION)

# _mir.so installs at site-packages root (not inside mir/) — matches setup.py Extension("_mir")
install(TARGETS _mir DESTINATION ${MIR_PYTHON_INSTALL_DIR} COMPONENT python)
install(FILES mir/src/mir/__init__.py DESTINATION ${MIR_PYTHON_INSTALL_DIR}/mir COMPONENT python)
