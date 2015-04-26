ecbuild_add_executable(
		TARGET      mir_tool
		SOURCES     mir_tool.cc
		INCLUDES    ${ECKIT_INCLUDE_DIRS}
		LIBS        mir ${ECKIT_LIBRARIES} )
