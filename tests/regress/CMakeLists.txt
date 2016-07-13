
# Download interpolation source files/reference results

file(STRINGS "download_tests_regress_src_files.txt" files)
ecbuild_get_test_multidata(
    TARGET download_tests_regress_src_files NOCHECK
    NAMES ${files} )

file(STRINGS "download_tests_regress_ref_files.txt" files)
ecbuild_get_test_multidata(
    TARGET download_tests_regress_ref_files NOCHECK
    NAMES ${files} )


######################### EMOS-216 #########################

foreach( area "" "_sub-area" )
  foreach( in "vo-d_sh" )
    foreach( out "regular_ll" "F80" "N80" "O80" )
      set( label "regress_EMOS-216_intuvp2_${in}_to_${out}${area}" )
      set( input "${in}.grib" )
      set( output "${label}.grib" )
      if( NOT (label MATCHES "_sub-area_to_" ) )
        unset( opt )
        if( out MATCHES "^regular_ll" )
          list( APPEND opt "--grid=3/3" )
        else()
          list( APPEND opt "--gridname=${out}" )
        endif()
        if( "${area}" STREQUAL "_sub-area" )
          list( APPEND opt "--area=70/-60/20/60" )
        endif()

        interpolation_add_test_interpol( ${label} "${input}" "${output}" "${opt}" download_tests_regress_src_files )
        interpolation_add_test_compare( ${label} "${output}" "${output}.ref"      download_tests_regress_ref_files )

        unset( opt )
      endif()
      unset( output )
      unset( input )
      unset( label )
    endforeach()
  endforeach()
endforeach()

foreach( area "" "_sub-area" )
  foreach( rotated "" "rotated_" "hirlam_non-rotated_" )
    foreach( in "2t_F640" "2t_F640_sub-area" "2t_N640" "2t_O1280" "2t_regular_ll" "2t_regular_ll_shifted" "2t_regular_ll_sub-area" "frpfire_regular_ll_shifted" "msl_F640" "msl_F640_sub-area" "msl_N640" "msl_O1280" "msl_regular_ll" "msl_regular_ll_shifted" "msl_regular_ll_sub-area" "swh_reduced_ll" "swh_reduced_ll_sub-area" "u-v_sh" "z_sh" )
      foreach( out "regular_ll" "F80" "N80" "O80" "F48" )
        set( label "regress_EMOS-216_intf2_${in}_to_${rotated}${out}${area}" )
        set( input "${in}.grib" )
        set( output "${label}.grib" )
        if( out MATCHES "^regular_ll" )
          list( APPEND opt "--grid=3/3" )
        else()
          list( APPEND opt "--gridname=${out}" )
        endif()
        if( "${rotated}" STREQUAL "hirlam_non-rotated_" )
          list( APPEND opt "--rotation=-90/0" )
        elseif( "${rotated}" STREQUAL "rotated_" )
          list( APPEND opt "--rotation=30/30" )
        endif()
        if( "${area}" STREQUAL "_sub-area" )
          list( APPEND opt "--area=70/-60/20/60" )
        endif()

        interpolation_add_test_interpol( ${label} "${input}" "${output}" "${opt}" download_tests_regress_src_files )
        interpolation_add_test_compare( ${label} "${output}" "${output}.ref"      download_tests_regress_ref_files )

        unset( opt )
        unset( output )
        unset( input )
        unset( label )
      endforeach()
    endforeach()
  endforeach()
endforeach()


######################### EMOS-260 #########################

set( label "regress_EMOS-260_intf2_2t_N640_to_regular_ll_sub-area" )
set( input "2t_N640.grib" )
set( output "${label}.grib" )
set( opt "--area=47.1/-13.05/33/8.1" "--grid=0.15/0.15" )

interpolation_add_test_interpol( ${label} "${input}" "${output}" "${opt}" download_tests_regress_src_files )
interpolation_add_test_compare( ${label} "${output}" "${output}.ref"      download_tests_regress_ref_files )

unset( opt )
unset( output )
unset( input )
unset( label )


######################### EMOS-262 #########################

set( label "regress_EMOS-262_intf2_msl_N640_to_regular_ll_sub-area_single-point" )
set( input "msl_N640.grib" )
set( output "${label}.grib" )
set( opt "--area=45.5/9.25/45.5/9.25" "--grid=0.125/0.125" )

interpolation_add_test_interpol( ${label} "${input}" "${output}" "${opt}" download_tests_regress_src_files )
interpolation_add_test_compare( ${label} "${output}" "${output}.ref"      download_tests_regress_ref_files )

unset( opt )
unset( output )
unset( input )
unset( label )


################### GRIB-863 / EMOS-214 ####################

# GRIB1 interpolate to regular_ll (1/16)/(1/16) degree
foreach( in "2t_O1280" )
  set( label "regress_GRIB-863_intf2_${in}_to_regular_ll_1-16" )
  set( input "${in}.grib" )
  set( output "${label}.grib" )
  set( opt "--grid=0.0625/0.0625" )

  interpolation_add_test_interpol( ${label} "${input}" "${output}" "${opt}" download_tests_regress_src_files )
  interpolation_add_test_compare( ${label} "${output}" "${output}.ref"      download_tests_regress_ref_files )

  unset( opt )
  unset( output )
  unset( input )
  unset( label )
endforeach()


######################### GRIB-864 #########################

# GRIB1 sub-milli degree encoding limit
# snap-milli-degree:     snaps to 0.001 because grid is 0.00025/0.00025, and is GRIB1-encodable
# snap-sub-milli-degree: snaps to 0.0005, not GRIB1-encodable (failure expected)
foreach( snap "snap-milli-degree" "snap-sub-milli-degree_bad" "snap-sub-milli-degree_sub-area_bad" )
  set( label "regress_GRIB-864_intf2_msl_N640_to_regular_ll_${snap}" )
  set( input "msl_N640.grib" )
  set( output "${label}.grib" )
  if(     snap STREQUAL "snap-sub-milli-degree_sub-area_bad" )
    set( opt "--grid=0.0625/0.0625"   "--area=1/0.0625/0/0.5625" )
  elseif( snap STREQUAL "snap-sub-milli-degree_bad" )
    set( opt "--grid=0.00025/0.00025" "--area=0.00074/0/0/0.00074" )
  else()
    set( opt "--grid=0.00025/0.00025" "--area=0.00076/0/0/0.00076" )
  endif()

  interpolation_add_test_interpol( ${label} "${input}" "/dev/null" "${opt}" download_tests_regress_src_files )
  if( snap MATCHES "sub-milli-degree" )
    set_tests_properties( ${label}_interpol PROPERTIES WILL_FAIL TRUE )
  endif()

  unset( opt )
  unset( output )
  unset( input )
  unset( label )
endforeach()

# FIXME: throws exception
# std::bad_alloc Caught in /home/ma/mapm/git/eckit/src/eckit/runtime/Tool.cc +80
foreach( snap "snap-milli-degree" )
  set( label "regress_GRIB-864_intf2_msl_N640_to_regular_ll_${snap}" )
  set_tests_properties(  ${label}_interpol PROPERTIES WILL_FAIL TRUE )
  ecbuild_warn(  "FIXME: ${label}_interpol avoids throwing exception" )
  unset( label )
endforeach()


############################################################

