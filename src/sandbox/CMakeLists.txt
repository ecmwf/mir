
foreach(tool IN ITEMS
        grids
        lsm)
    if(HAVE_BUILD_TOOLS)
        ecbuild_add_executable(TARGET ${tool} SOURCES ${tool}.cc LIBS mir)
    else()
        ecbuild_add_executable(TARGET ${tool} SOURCES ${tool}.cc LIBS mir NOINSTALL)
    endif()
endforeach()

