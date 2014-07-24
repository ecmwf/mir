#!/bin/ksh
. ../error_codes.sh
. ../utils.sh

if [[ $# -ne 1 ]]; then
  echo "Usage: scriptname.sh XML_FILE"
  exit 1
fi

file=$1

typeset -i interpol_mem_0
typeset -i interpol_mem_1

set -A interpol_mems $(xpath_values $file '//Test/Interpolation/@MemoryBytes')
# do comparisons between the two memory usages


if [ ${#interpol_mems[@]} -eq 2 ]; then

  interpol_mem_0=${interpol_mems[0]}
  interpol_mem_1=${interpol_mems[1]}

  # do comparisons between the two memory measurements
  # increase in memory over would be considered bad

  if [[ $interpol_mem_1 -gt 1.1*$interpol_mem_0 ]]; then
    exit $TEST_RESOURCE_MAJOR
  fi

  if [[ $interpol_mem_0 -gt 1.1*$interpol_mem_1 ]]; then
    exit $TEST_RESOURCE_MAJOR
  fi

fi

exit $TEST_PASS

