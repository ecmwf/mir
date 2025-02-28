file(GLOB config_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.yaml")

install(
    FILES       ${config_files}
    DESTINATION "etc/mir"
    PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ)

foreach(_f ${config_files})
    configure_file(${_f} "${CMAKE_BINARY_DIR}/etc/mir/${_f}" COPYONLY)
endforeach()
