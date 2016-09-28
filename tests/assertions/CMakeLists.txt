ecbuild_find_python( VERSION 2.7 )

file(GLOB_RECURSE test_files LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_BINARY_DIR} *.test)

# set(MIR_TOOL     "$<TARGET_FILE:mir-tool>")
# set(MIR_COMPARE  "$<TARGET_FILE:mir-compare>")
# set(GRIB_GET     "$<TARGET_FILE:grib_get>")
# set(GRIB_COMPARE "$<TARGET_FILE:grib_compare>")

file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/mir-test.py INPUT  ${CMAKE_CURRENT_SOURCE_DIR}/mir-test.py )

ecbuild_info(${test_files})

foreach( _t ${test_files} )

  ecbuild_add_test(

      TYPE       PYTHON

      COMMAND    mir-test.py

      ARGS       ${_t}

  )

endforeach()


