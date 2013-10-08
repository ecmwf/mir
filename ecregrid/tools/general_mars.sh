#!/bin/sh

set -eaux

#export MARS_DEBUGGER=valgrind
MARS_TEST_PATH=/vol/marsdev/data/build/linux/ia32/opensuse103/mars/client
export MARS_TEST_PATH

# mars with grib_api
#MARS="mars -g"
#MARS=mars

MARS="mars -p"
MARS="mars -t"
MARS="mars"

in=$1
out=$2
gr=$3

$MARS <<@@
read,
#	style=dissemination,
	$gr
	source="$in",
	target="$out"
@@

