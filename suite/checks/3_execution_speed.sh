#!/bin/ksh
. ../error_codes.sh
. ../utils.sh

if [[ $# -ne 1 ]]; then
  echo "Usage: scriptname.sh XML_FILE"
  exit 1
fi
file=$1

typeset -i interpol_time_0
typeset -i interpol_time_1

set -A interpol_times $(xpath_values $file '//Test/Interpolation/@TimerMs')
# do comparisons between the two execution times

if [ ${#interpol_times[@]} -eq 2 ]; then

  interpol_time_0=${interpol_times[0]}
  interpol_time_1=${interpol_times[1]}

  if [[ $interpol_time_0 -gt 1.5*$interpol_time_1 ]]; then
    exit $TEST_RESOURCE_MAJOR
  fi

  if [[ $interpol_time_1 -gt 1.5*$interpol_time_0 ]]; then
    exit $TEST_RESOURCE_MAJOR
  fi

  if [[ $interpol_time_0 -gt 1.1*$interpol_time_1 ]]; then
    exit $TEST_RESOURCE_MINOR
  fi

  if [[ $interpol_time_1 -gt 1.1*$interpol_time_0 ]]; then
    exit $TEST_RESOURCE_MINOR
  fi
fi
exit $TEST_PASS

