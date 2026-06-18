# (C) Copyright 1996- ECMWF.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation nor
# does it submit to any jurisdiction.

# Require Cython; Install destination: prefix-relative site-packages
execute_process(
    COMMAND ${Python3_EXECUTABLE} -c "import cython; import sysconfig; print(sysconfig.get_path('platlib', vars={'platbase': '${CMAKE_INSTALL_PREFIX}', 'base': '${CMAKE_INSTALL_PREFIX}'}))"
    OUTPUT_VARIABLE _python_sitearch
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY
)

set(_src ${CMAKE_CURRENT_SOURCE_DIR}/mir/src/_mir)
set(PYTHONEXT_INSTALL_DIR ${_python_sitearch} CACHE PATH "Install directory for Python extensions")
set(_cython_flags
    --cplus
    --directive language_level=3
    --directive c_string_type=unicode
    --directive c_string_encoding=utf8
)

# ---- mir._mir ----
set(_mir_cpp ${CMAKE_CURRENT_BINARY_DIR}/_mir.cpp)

add_custom_command(
    OUTPUT ${_mir_cpp}
    COMMAND
        ${Python3_EXECUTABLE} -m cython
        ${_cython_flags}
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
    ${_src}/mir/appendix.cc
    ${_src}/mir/input/ArrayInput.cc
    ${_src}/mir/input/PyGribInput.cc
    ${_src}/mir/output/PyGribOutput.cc
)

target_include_directories(_mir PRIVATE ${PROJECT_SOURCE_DIR}/src ${PROJECT_BINARY_DIR}/src ${_src})
target_link_libraries(_mir PRIVATE mir Python3::NumPy)
target_compile_definitions(_mir PRIVATE NPY_NO_DEPRECATED_API=NPY_1_7_API_VERSION)

install(TARGETS _mir          DESTINATION ${PYTHONEXT_INSTALL_DIR}/mir COMPONENT python)
install(DIRECTORY mir/src/mir/ DESTINATION ${PYTHONEXT_INSTALL_DIR}/mir COMPONENT python FILES_MATCHING PATTERN "*.py")

file(GLOB _tools RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} mir/src/mir/tools/*.py)
foreach(_tool IN LISTS _tools)
    get_filename_component(_tool_we ${_tool} NAME_WE)
    if(_tool_we STREQUAL "__init__")
        continue()
    endif()
    string(REPLACE "_" "-" _tool_we ${_tool_we})
    install(PROGRAMS ${_tool} DESTINATION bin RENAME mir-${_tool_we} COMPONENT python)
endforeach()
