#!/bin/ksh

. ../error_codes.sh
. ../utils.sh

if [[ $# -ne 1 ]]; then
  echo "Usage: scriptname.sh XML_FILE"
  exit 1
fi

file=$1

if [[ ! -f $file ]]; then
  echo "File $file does not exist"
  exit $TEST_FAIL
fi

set -A files $(xpath_values $file '//Test/Interpolation/Output[@Type="GRIB"]/@File')
echo "There are ${#files[*]} GRIB files defined in $file"
# read the folder path here
folder=`xpath_values $file '//Test/@Folder'`


if [[ ${#files[*]} -eq 2 ]]; then

  echo "Within IF statement for two files"
  file0=${folder}/${files[0]}
  file1=${folder}/${files[1]}

  echo $file0
  echo $file1
  rtn=`grib_compare -f -Rvalues=0.05 -cvalues $file0 $file1`
  echo "The output of the grib_compare call with tolerance is '$rtn'"
   
  if [[ $rtn != "" ]]; then
    echo "non-null output from grib_compare diff > tolerance. returning major warning"
    exit $TEST_FILE_DIFFERENCE_MAJOR
  fi

  rtn=`grib_compare -f -Rvalues=0.0 -cvalues $file0 $file1`
  echo "The output of the grib_compare call without tolerance is '$rtn'"
   
  if [[ $rtn != "" ]]; then
    echo "non-null output from grib_compare but < tolerance. returning minor warning"
    exit $TEST_FILE_DIFFERENCE_MINOR
  fi

  echo "null output from grib_compare indicating exact match. returning pass"
  exit $TEST_PASS

fi

# If here, we didn't have two output grib files. should have been picked up
# elsewhere but fail in any case
echo "One or more output files were not found. Retuning fail"
exit $TEST_FAIL


~
~



