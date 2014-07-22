#!/bin/ksh
. ../error_codes.sh
. ../utils.sh

if [[ $# -ne 1 ]]; then
  echo "Usage: scriptname.sh XML_FILE"
  exit 1
fi
file=$1

typeset -i ecregrid_time
typeset -i emos_time

ecregrid_time=$(xpath_values $file '//Test/Interpolation[@Name="ecregrid"]/@TimerMs')
emos_time=$(xpath_values $file '//Test/Interpolation[@Name="emos"]/@TimerMs')

# do comparisons between ecregrid and emos execution times

if [[ $ecregrid_time -gt 1.05*$emos_time ]]; then
  exit $TEST_RESOURCE_MAJOR
fi

if [[ $ecregrid_time -gt $emos_time ]]; then
  exit $TEST_RESOURCE_MINOR
fi

exit $TEST_PASS

