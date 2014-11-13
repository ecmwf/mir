ecbuild_add_executable(
		TARGET      mir_interpolate
		SOURCES     mir_interpolate.cc
		INCLUDES    ${ATLAS_INCLUDE_DIRS} ${ECKIT_INCLUDE_DIRS}
		LIBS        mir ${ATLAS_LIBRARIES} ${ECKIT_LIBRARIES} )

ecbuild_add_executable(
		TARGET      mir_transform
		SOURCES     mir_transform.cc
		INCLUDES    ${ATLAS_INCLUDE_DIRS} ${ECKIT_INCLUDE_DIRS}
		LIBS        mir ${ATLAS_LIBRARIES} ${ECKIT_LIBRARIES} )

