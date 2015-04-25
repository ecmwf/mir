set( _mir_test_environment MIR_CACHE_DIR=$ {CMAKE_CURRENT_BINARY_DIR}/cache )


# ecbuild_add_test(
#     TARGET    soyuz
#     SOURCES   soyuz.cc
#     RESOURCES test.bitmap test.grib test.nc sh.grib
#     INCLUDES  ${ECKIT_INCLUDE_DIRS} ${NETCDF_INCLUDE_DIRS}
#     LIBS      mir
# )

ecbuild_add_test(
    TARGET    spherical_to_gaussian
    SOURCES   spherical_to_gaussian.cc
    RESOURCES sh.grib
    INCLUDES  ${ECKIT_INCLUDE_DIRS}
    LIBS      mir
)

ecbuild_add_test(
    TARGET    spherical_to_spherical
    SOURCES   spherical_to_spherical.cc
    RESOURCES sh.grib
    INCLUDES  ${ECKIT_INCLUDE_DIRS}
    LIBS      mir
)

ecbuild_add_test(
    TARGET    reduced_gg_to_regular_gg
    SOURCES   reduced_gg_to_regular_gg.cc
    RESOURCES reduced_gg.grib
    INCLUDES  ${ECKIT_INCLUDE_DIRS}
    LIBS      mir
)

ecbuild_add_test(
    TARGET    reduced_gg_to_regular_ll
    SOURCES   reduced_gg_to_regular_ll.cc
    RESOURCES reduced_ll.grib
    INCLUDES  ${ECKIT_INCLUDE_DIRS}
    LIBS      mir
)
