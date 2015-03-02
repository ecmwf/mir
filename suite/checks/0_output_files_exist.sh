#!/bin/ksh

. ../error_codes.sh
. ../utils.sh

if [[ $# -ne 1 ]]; then
  echo "Usage: scriptname.sh XML_FILE"
  exit 1
fi

file=$1
# read the folder path here
folder=`xpath_values $file '//Test/@Folder'`

if [[ ! -f $file ]]; then
  echo "File $file does not exist"
  exit $TEST_FAIL
fi

set -A files $(xpath_values $file '//Test/Interpolation/Output/@File')
echo "There are ${#files[*]} files defined in $file"

for file in ${files[@]}; do
  testfile=${folder}/${file}
  if [[ ! -f $testfile ]]; then
    echo "File $testfile does NOT exist"
    exit $TEST_FAIL
  fi
  echo "File $testfile exists"
done


exit $TEST_PASS


~
~



