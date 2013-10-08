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

files="spectral_511.grib1"

for r in ${files[*]}; do

  requestfile=${r%\.*}.mars

  ksh ./get_source_request.sh $data/$r $requestfile

  echo "-----------------------------"
  echo "SH to LL ... "
  echo "-----------------------------"

  outputname=y_$r-latlon_0.25_0.25

  # add target info to request
  targetrequest=${outputname}.mars
  targetgrib=${outputname}.grib
  cp -f $requestfile $targetrequest

  echo "grid = $ll1," >> $targetrequest
  echo "target='$out/$targetgrib'" >> $targetrequest

  $MARS < $targetrequest

  echo "grib_compare -P $out/$targetgrib $data_ref/$targetgrib"
  grib_compare -P $out/$targetgrib $data_ref/$targetgrib

  echo "-----------------------------"
  echo "SH to LL done  "
  echo "-----------------------------"


done
