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

ecbuild_add_executable(
		TARGET      mir_config
		SOURCES     mir_config.cc
		INCLUDES    ${ECKIT_INCLUDE_DIRS}
		LIBS        mir ${ECKIT_LIBRARIES} )

ecbuild_add_executable(
		TARGET      mir_shared_memory
		SOURCES     mir_shared_memory.cc
		INCLUDES    ${ECKIT_INCLUDE_DIRS}
		LIBS        mir ${ECKIT_LIBRARIES} )

ecbuild_add_executable(
		TARGET      mir_make_lsm
		SOURCES     mir_make_lsm.cc
		INCLUDES    ${ECKIT_INCLUDE_DIRS}
		LIBS        mir ${ECKIT_LIBRARIES} )

ecbuild_add_executable(
		TARGET      mir_plot_lsm
		SOURCES     mir_plot_lsm.cc
		INCLUDES    ${ECKIT_INCLUDE_DIRS}
		LIBS        mir ${ECKIT_LIBRARIES} )

########################################################################################################################

# Just for testing and validation

ecbuild_add_executable(
        TARGET      emos_tool
        SOURCES     emos_tool.cc
        CONDITION   HAVE_LIBEMOS_INTERPOLATION
        INCLUDES    ${ECKIT_INCLUDE_DIRS} ${GRIB_API_INCLUDE_DIRS}
        LIBS        ${ECKIT_LIBRARIES} ${LIBEMOS_LIBRARIES} ${GRIB_API_LIBRARIES} )

