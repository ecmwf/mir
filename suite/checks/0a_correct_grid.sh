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

target_grid=$(xpath_values $file '//Test/TargetGrid/@ShortName')

if [[ ${#files[*]} -eq 2 ]]; then

  echo "Within IF statement for two files"
  file0=${folder}/${files[0]}
  file1=${folder}/${files[1]}

  echo $file0
  echo $file1


  # check first message
  grid0=`grib_get -w count=1 -p gridType ${file0}`
  grid1=`grib_get -w count=1 -p gridType ${file1}`
   
  if [[ $grid0 == $grid1 ]]; then
    echo "both files contain same grid type ($grid0)"

    # need to reformat rot_reg to rotated
    #                  red to reduced
    #                  reg to regular
    
    suite_target_grid=$target_grid # preserve terminology used in suite
    target_grid=`echo $target_grid | sed "s/rot_reg/rotated/g"`
    target_grid=`echo $target_grid | sed "s/reg/regular/g"`
    target_grid=`echo $target_grid | sed "s/red/reduced/g"`

    if [[ $grid0 != $target_grid ]]; then
      echo "wrong target grid ($grid0 c.w. $target_grid [$suite_target_grid])"
      exit $TEST_WRONG_TARGET_GRID
    fi
    exit $TEST_PASS
  else
    # failure condition
    echo "There are different target grids in our output files"
    exit $TEST_WRONG_TARGET_GRID
  fi

fi

# If here, we didn't have two output grib files. should have been picked up
# elsewhere but fail in any case
echo "Failed to find two output files. Returning fail"
exit $TEST_FAIL


~
~



