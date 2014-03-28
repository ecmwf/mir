### config headers

ecbuild_generate_config_headers( DESTINATION ${INSTALL_INCLUDE_DIR}/mir )

configure_file( mir_config.h.in   mir_config.h  )
configure_file( mir_version.h.in  mir_version.h )

install(FILES
			${CMAKE_CURRENT_BINARY_DIR}/mir_config.h
			${CMAKE_CURRENT_BINARY_DIR}/mir_version.h
		DESTINATION
			${INSTALL_INCLUDE_DIR}/mir )

configure_file( mir_version.cc.in  mir_version.cc )

### mir sources

list( APPEND mir_srcs mir.h mir_version.cc )

list( APPEND mir_srcs
Action.cc
Action.h
FEInterpolator.h
FEInterpolator.cc
Interpolator.cc
Interpolator.h
PointSearch.cc
PointSearch.h
Weights.cc
Weights.h
WeightCache.cc
WeightCache.h
 )


ecbuild_add_library(TARGET		mir
					INSTALL_HEADERS LISTED
					HEADER_DESTINATION
						${INSTALL_INCLUDE_DIR}/mir
					SOURCES		${mir_srcs}
					GENERATED	mir_version.cc
					LIBS		${ATLAS_LIBRARIES} ${ECKIT_LIBRARIES} )


