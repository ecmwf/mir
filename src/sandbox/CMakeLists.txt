ecbuild_add_executable( TARGET  vod2uv
                        SOURCES  vod2uv.cc
                        INCLUDES ${ECKIT_INCLUDE_DIRS}
                        LIBS     mir ${ECKIT_LIBRARIES})

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
