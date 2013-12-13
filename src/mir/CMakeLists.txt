list( APPEND mir_srcs
Action.cc
Action.h
Interpolator.cc
Interpolator.h
PointSearch.cc
PointSearch.h
Weights.cc
Weights.h
 )


ecbuild_add_library(TARGET mir
                    SOURCES ${mir_srcs}
                    LIBS eckit )


