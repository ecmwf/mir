#include( ecbuild_system )

# Make sure the compiler can find include files from our Hello library.
include_directories (${ECREGRID_SOURCE_DIR}/src ${GRIB_API_INCLUDE_DIRS})

# Make sure the linker can find the Hello library once it is built.
link_directories (${ECREGRID_BINARY_DIR}/src)

# Add executable called "helloDemo" that is built from the source files
ecbuild_add_executable (  TARGET   ecRegrid-tool 
                          SOURCES  HandleRequest.cpp ecregrid.cpp)

# Link the executable to the Hello library.
target_link_libraries (ecRegrid-tool ecregrid ${GRIB_API_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

SET_TARGET_PROPERTIES(ecRegrid-tool PROPERTIES OUTPUT_NAME "ecregrid")
