ecbuild_add_executable(
		TARGET      mir_tool
		SOURCES     mir_tool.cc
		INCLUDES    ${ATLAS_INCLUDE_DIRS}
    LIBS        mir )

ecbuild_add_executable(
		TARGET      mir_inspect
		SOURCES     mir_inspect.cc
		INCLUDES    ${ATLAS_INCLUDE_DIRS}
    LIBS        mir )

ecbuild_add_executable(
		TARGET      mir_compare
		SOURCES     mir_compare.cc
		INCLUDES    ${ATLAS_INCLUDE_DIRS}
    LIBS        mir )

ecbuild_add_executable(
		TARGET      mir_config
		SOURCES     mir_config.cc
		INCLUDES    ${ATLAS_INCLUDE_DIRS}
    LIBS        mir )

ecbuild_add_executable(
		TARGET      mir_shared_memory
		SOURCES     mir_shared_memory.cc
		INCLUDES    ${ATLAS_INCLUDE_DIRS}
    LIBS        mir )

ecbuild_add_executable(
		TARGET      mir_make_lsm
		SOURCES     mir_make_lsm.cc
		INCLUDES    ${ATLAS_INCLUDE_DIRS}
    LIBS        mir )

ecbuild_add_executable(
		TARGET      mir_plot_lsm
		SOURCES     mir_plot_lsm.cc
		INCLUDES    ${ATLAS_INCLUDE_DIRS}
    LIBS        mir )

ecbuild_add_executable(
        TARGET      mir_stats
        SOURCES     mir_stats.cc
        INCLUDES    ${ATLAS_INCLUDE_DIRS}
        LIBS        mir )

ecbuild_add_executable(
        TARGET      mir_integrate
        SOURCES     mir_integrate.cc
        INCLUDES    ${ATLAS_INCLUDE_DIRS}
        LIBS        mir )
