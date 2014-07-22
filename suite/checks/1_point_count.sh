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

  typeset -i points0
  typeset -i points1

  # check first message
  points0=`grib_get -w count=1 -p numberOfDataPoints ${file0}`
  points1=`grib_get -w count=1 -p numberOfDataPoints ${file1}`
   
  length0=`grib_get -w count=1 -p totalLength ${file0}`
  length1=`grib_get -w count=1 -p totalLength ${file1}`

  if [[ $points0 == $points1 && $length0 == $length1 ]]; then
    echo "both files contain equal numberOfDataPoints ($points0 and $points1)"
    echo "and both files contain equal totalLength ($length0 and $length1)"
    exit $TEST_PASS
  else
    # failure condition
    echo "There are different numberOfDataPoints or totalLength in our output files"
    exit $TEST_FILE_INTEGRITY_FAIL
  fi

fi

# If here, we didn't have two output grib files. should have been picked up
# elsewhere but fail in any case
echo "Failed to find two output files. Returning fail"
exit $TEST_FAIL


~
~



