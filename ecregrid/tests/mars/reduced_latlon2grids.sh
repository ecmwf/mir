#!/bin/ksh

. ./common.sh


area1="90.0/-180.0/-90.0/180.0"
area2="90.0/0/-90.0/360"
area3="80.0/-40.0/20.0/60.0"
area4="60.0/-10.0/10.0/15.0"
area5="60.0/10.0/10.0/35.0"

ll1=0.25/0.25
ll2=1/1
ll3=0.5/0.5
gg1=200 
gg2=400 
rot1="-30/10"
sh=80

files="surface_latlon_reduced.grib1"

for r in ${files[*]}; do

  requestfile=${r%\.*}.mars

  ksh ./get_source_request.sh $data/$r $requestfile

  echo "-----------------------------"
  echo "RL to LL Subarea 1 ..."
  echo "-----------------------------"

  outputname=y_$r-ll_subarea1

  # add target info to request
  targetrequest=${outputname}.mars
  targetgrib=${outputname}.grib
  cp -f $requestfile $targetrequest

  ll=$ll2
  a=$area3
  echo "grid = $ll," >> $targetrequest
  echo "area = $a," >> $targetrequest
  echo "target='$out/$targetgrib'" >> $targetrequest

  $MARS < $targetrequest

  echo "grib_compare -P $out/$targetgrib $data_ref/$targetgrib"
  grib_compare -P $out/$targetgrib $data_ref/$targetgrib

  echo "-----------------------------"
  echo "RL to LL Subarea 1 done"
  echo "-----------------------------"

  echo "-----------------------------"
  echo "RL to LL Subarea 2 ..."
  echo "-----------------------------"

  outputname=y_$r-ll_subarea2

  # add target info to request
  targetrequest=${outputname}.mars
  targetgrib=${outputname}.grib
  cp -f $requestfile $targetrequest

  ll=$ll2
  a=$area4
  echo "grid = $ll," >> $targetrequest
  echo "area = $a," >> $targetrequest
  echo "target='$out/$targetgrib'" >> $targetrequest

  $MARS < $targetrequest

  echo "grib_compare -P $out/$targetgrib $data_ref/$targetgrib"
  grib_compare -P $out/$targetgrib $data_ref/$targetgrib

  echo "-----------------------------"
  echo "RL to LL Subarea 2 done"
  echo "-----------------------------"

  echo "-----------------------------"
  echo "RL to LL Subarea 3 ..."
  echo "-----------------------------"

  outputname=y_$r-ll_subarea3

  # add target info to request
  targetrequest=${outputname}.mars
  targetgrib=${outputname}.grib
  cp -f $requestfile $targetrequest

  ll=$ll1
  a=$area1
  echo "grid = $ll," >> $targetrequest
  echo "area = $a," >> $targetrequest
  echo "target='$out/$targetgrib'" >> $targetrequest

  $MARS < $targetrequest

  echo "grib_compare -P $out/$targetgrib $data_ref/$targetgrib"
  grib_compare -P $out/$targetgrib $data_ref/$targetgrib

  echo "-----------------------------"
  echo "RL to LL Subarea 3 done"
  echo "-----------------------------"
  
  echo "-----------------------------"
  echo "RL to LL ..."
  echo "-----------------------------"

  outputname=y_$r-ll

  # add target info to request
  targetrequest=${outputname}.mars
  targetgrib=${outputname}.grib
  cp -f $requestfile $targetrequest

  ll=$ll1
  echo "grid = $ll," >> $targetrequest
  echo "target='$out/$targetgrib'" >> $targetrequest

  $MARS < $targetrequest

  echo "grib_compare -P $out/$targetgrib $data_ref/$targetgrib"
  grib_compare -P $out/$targetgrib $data_ref/$targetgrib

  echo "-----------------------------"
  echo "RL to LL done"
  echo "-----------------------------"

  echo "-----------------------------"
  echo "RL to LL Rotated..."
  echo "-----------------------------"

  outputname=y_$r-llrot

  # add target info to request
  targetrequest=${outputname}.mars
  targetgrib=${outputname}.grib
  cp -f $requestfile $targetrequest

  ll=$ll1
  rot=$rot1
  echo "grid = $ll," >> $targetrequest
  echo "rotation = $rot," >> $targetrequest
  echo "target='$out/$targetgrib'" >> $targetrequest

  $MARS < $targetrequest

  echo "grib_compare -P $out/$targetgrib $data_ref/$targetgrib"
  grib_compare -P $out/$targetgrib $data_ref/$targetgrib

  echo "-----------------------------"
  echo "RL to LL Rotated done"
  echo "-----------------------------"


  echo "-----------------------------"
  echo "RL to GG..."
  echo "-----------------------------"

  outputname=y_$r-gg

  # add target info to request
  targetrequest=${outputname}.mars
  targetgrib=${outputname}.grib
  cp -f $requestfile $targetrequest

  gg=$gg2
  echo "grid = $gg," >> $targetrequest
  echo "target='$out/$targetgrib'" >> $targetrequest

  $MARS < $targetrequest

  echo "grib_compare -P $out/$targetgrib $data_ref/$targetgrib"
  grib_compare -P $out/$targetgrib $data_ref/$targetgrib

  echo "-----------------------------"
  echo "RL to GG done"
  echo "-----------------------------"



done

