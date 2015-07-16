## TODO executable vod2uv fails to link with Intel compiler
##      due to name mangling of intout_ function in libemos.cc

ecbuild_add_executable( TARGET  vod2uv
                        SOURCES  vod2uv.cc
                        CONDITION NOT CMAKE_CXX_COMPILER_ID STREQUAL "Intel"
                        INCLUDES ${ECKIT_INCLUDE_DIRS}
                        LIBS     mir ${LIBEMOS_LIBRARIES} ${ECKIT_LIBRARIES})

ecbuild_add_executable( TARGET  grids
                        SOURCES  grids.cc
                        INCLUDES ${ECKIT_INCLUDE_DIRS}
                        LIBS     mir ${LIBEMOS_LIBRARIES} ${ECKIT_LIBRARIES})

ecbuild_add_executable( TARGET  lsm
                        SOURCES  lsm.cc
                        INCLUDES ${ECKIT_INCLUDE_DIRS}
                        LIBS     mir ${LIBEMOS_LIBRARIES} ${ECKIT_LIBRARIES})

ecbuild_add_executable( TARGET  json_config
                        SOURCES  json_config.cc
                        INCLUDES ${ECKIT_INCLUDE_DIRS}
                        LIBS     mir ${LIBEMOS_LIBRARIES} ${ECKIT_LIBRARIES})

# ecbuild_add_executable( TARGET  eigen_sparse
# 						SOURCES eigen_sparse.cpp utils.h
# 						CONDITION ECKIT_FOUND AND HAVE_SANDBOX )
#
# ecbuild_add_executable( TARGET  trilinos_sparse
# 						SOURCES trilinos_sparse.cpp utils.h
# 						LIBS    ${TRILINOS_LIBRARIES}
# 						CONDITION TRILINOS_FOUND AND SANDBOX )
#
# ecbuild_add_executable( TARGET  params
# 						SOURCES params.cpp
# 						CONDITION ECKIT_FOUND AND HAVE_SANDBOX )
#
