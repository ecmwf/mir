ecbuild_add_executable( TARGET   ecregrid_tool
						SOURCES  HandleRequest.cpp ecregrid_tool.cpp)

target_link_libraries( ecregrid_tool ecregrid ${GRIB_API_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} )

set_target_properties( ecregrid_tool PROPERTIES OUTPUT_NAME "ecregrid")
