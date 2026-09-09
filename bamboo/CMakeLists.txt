# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

file(GLOB_RECURSE bamboo_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*")

ecbuild_add_resources(TARGET ${PROJECT_NAME}_bamboo SOURCES_DONT_PACK ${bamboo_files})
