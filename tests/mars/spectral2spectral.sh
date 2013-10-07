#!/bin/ksh

. ./common.sh


sh1=319
sh2=799

files="spectral_511.grib1"

for r in ${files[*]}; do

  requestfile=${r%\.*}.mars

  ksh ./get_source_request.sh $data/$r $requestfile

  echo "-----------------------------"
  echo "SH to SH ... "
  echo "-----------------------------"

  sh=$sh1
  outputname=y_$r-sh_$sh

  # add target info to request
  targetrequest=${outputname}.mars
  targetgrib=${outputname}.grib
  cp -f $requestfile $targetrequest

  echo "resol = $sh," >> $targetrequest
  echo "target='$out/$targetgrib'" >> $targetrequest

  $MARS < $targetrequest

  echo "grib_compare -P $out/$targetgrib $data_ref/$targetgrib"
  grib_compare -P $out/$targetgrib $data_ref/$targetgrib

  echo "-----------------------------"
  echo "SH to SH done  "
  echo "-----------------------------"


done


