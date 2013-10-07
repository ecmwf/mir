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

files="upper_air_gaussian_reduced.grib1"

for r in ${files[*]}; do

  requestfile=${r%\.*}.mars

  ksh ./get_source_request.sh $data/$r $requestfile

  echo ------------------------------
  echo "RG to LL Subarea 2 ..."
  echo ------------------------------
  ll=$ll2
  a=$area4
  outputname=y_$r-ll_subarea2

  # add target info to request
  targetrequest=${outputname}.mars
  targetgrib=${outputname}.grib
  cp -f $requestfile $targetrequest

  echo "grid = $ll," >> $targetrequest
  echo "area = $a," >> $targetrequest
  echo "interpolation = linearfit," >> $targetrequest
  echo "target='$out/$targetgrib'" >> $targetrequest

  $MARS < $targetrequest

  echo "grib_compare -P $out/$targetgrib $data_ref/$targetgrib"
  grib_compare -P $out/$targetgrib $data_ref/$targetgrib

  echo "-----------------------------"
  echo "RG to LL Subarea 2 done"
  echo "-----------------------------"


  echo ------------------------------
  echo "RG to LL Subarea 3 ..."
  echo ------------------------------
  ll=$ll1
  a=$area1
  outputname=y_$r-ll_subarea3

  # add target info to request
  targetrequest=${outputname}.mars
  targetgrib=${outputname}.grib
  cp -f $requestfile $targetrequest

  echo "grid = $ll," >> $targetrequest
  echo "area = $a," >> $targetrequest
  echo "interpolation = linearfit," >> $targetrequest
  echo "target='$out/$targetgrib'" >> $targetrequest

  $MARS < $targetrequest

  echo "grib_compare -P $out/$targetgrib $data_ref/$targetgrib"
  grib_compare -P $out/$targetgrib $data_ref/$targetgrib

  echo "-----------------------------"
  echo "RG to LL Subarea 3 done"
  echo "-----------------------------"


  echo ------------------------------
  echo "RG to LL ..."
  echo ------------------------------
  ll=$ll1
  outputname=y_$r-ll

  # add target info to request
  targetrequest=${outputname}.mars
  targetgrib=${outputname}.grib
  cp -f $requestfile $targetrequest

  echo "grid = $ll," >> $targetrequest
  echo "interpolation = linearfit," >> $targetrequest
  echo "target='$out/$targetgrib'" >> $targetrequest

  $MARS < $targetrequest

  echo "grib_compare -P $out/$targetgrib $data_ref/$targetgrib"
  grib_compare -P $out/$targetgrib $data_ref/$targetgrib

  echo "-----------------------------"
  echo "RG to LL done"
  echo "-----------------------------"
  
  echo ------------------------------
  echo "RG to Rotated LL ..."
  echo ------------------------------
  ll=$ll1
  rot=$rot1
  outputname=y_$r-llrot

  # add target info to request
  targetrequest=${outputname}.mars
  targetgrib=${outputname}.grib
  cp -f $requestfile $targetrequest

  echo "grid = $ll," >> $targetrequest
  echo "rotation = $rot," >> $targetrequest
  echo "interpolation = cubic," >> $targetrequest
  echo "target='$out/$targetgrib'" >> $targetrequest

  $MARS < $targetrequest

  echo "grib_compare -P $out/$targetgrib $data_ref/$targetgrib"
  grib_compare -P $out/$targetgrib $data_ref/$targetgrib

  echo "-----------------------------"
  echo "RG to Rotated LL done"
  echo "-----------------------------"


  echo ------------------------------
  echo "RG to GG Subarea ..."
  echo ------------------------------
  gg=$gg2
  area=$area3
  outputname=y_$r-gg_subarea1

  # add target info to request
  targetrequest=${outputname}.mars
  targetgrib=${outputname}.grib
  cp -f $requestfile $targetrequest

  echo "grid = $gg2," >> $targetrequest
  echo "area = $area3," >> $targetrequest
  echo "interpolation = linearfit," >> $targetrequest
  echo "target='$out/$targetgrib'" >> $targetrequest

  $MARS < $targetrequest

  echo "grib_compare -P $out/$targetgrib $data_ref/$targetgrib"
  grib_compare -P $out/$targetgrib $data_ref/$targetgrib

  echo "-----------------------------"
  echo "RG to GG Subarea done"
  echo "-----------------------------"


done

