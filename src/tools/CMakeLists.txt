ecbuild_add_executable(
		TARGET      mir_tool
		SOURCES     mir_tool.cc
		INCLUDES    ${ECKIT_INCLUDE_DIRS}
		LIBS        mir ${ECKIT_LIBRARIES} )

ecbuild_add_executable(
		TARGET      mir_inspect
		SOURCES     mir_inspect.cc
		INCLUDES    ${ECKIT_INCLUDE_DIRS}
		LIBS        mir ${ECKIT_LIBRARIES} )

ecbuild_add_executable(
		TARGET      mir_compare
		SOURCES     mir_compare.cc
		INCLUDES    ${ECKIT_INCLUDE_DIRS}
		LIBS        mir ${ECKIT_LIBRARIES} )
