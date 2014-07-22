#!/bin/ksh
. ../error_codes.sh
. ../utils.sh

if [[ $# -ne 1 ]]; then
  echo "Usage: scriptname.sh XML_FILE"
  exit 1
fi

file=$1

typeset -i ecregrid_mem
typeset -i emos_mem

ecregrid_mem=$(xpath_values $file '//Test/Interpolation[@Name="ecregrid"]/@MemoryBytes')
emos_mem=$(xpath_values $file '//Test/Interpolation[@Name="emos"]/@MemoryBytes')

# do comparisons between ecregrid and emos memory checks
# ANY increase in memory over emos considered bad

echo "ecregrid memory is $ecregrid_mem"
echo "emos memory" $emos_mem

if [[ $ecregrid_mem -gt $emos_mem ]]; then
  exit $TEST_RESOURCE_MAJOR
fi

exit $TEST_PASS

exit

