#!/bin/sh

. `dirname $0`/include.sh
set -e

#export ECREGRID_DUMP_TO_FILE="./spectral2spectral_wind.dat"

compare="grib_compare -f"

valgrind=valgrind
valgrind="valgrind --leak-check=full"
valgrind=

sh1=319
sh1=799

echo "******************************"
echo "SH to SH WIND             ****"
echo "******************************"

files="vortdiv.grib"

for file in `echo $files`
do
#
echo "-----------------------------"
echo "SH to SH Wind ... "
echo "-----------------------------"
x=y_$file-sh_319.grib
$tool -i $data/$file -o $out/$x -t $sh1
$compare $data_ref/$x $out/$x
echo "-----------------------------"
echo "SH to SH Wind done"
echo "-----------------------------"
#
done
