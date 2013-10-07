#!/bin/sh

. `dirname $0`/include.sh
set -e

#export ECREGRID_DUMP_TO_FILE="./spectral2spectral.dat"

compare=grib_compare

valgrind=valgrind
valgrind="valgrind --leak-check=full"
valgrind=

sh1=319
sh2=799

echo "******************************"
echo "SH to SH                  ****"
echo "******************************"

files="spectral_511.grib1"

for file in `echo $files`
do
echo "-----------------------------"
echo "SH to SH ... "
echo "-----------------------------"
#
sh=$sh1
x=y_$file-sh_$sh.grib
$tool -i $data/$file -o $out/$x -t $sh
$compare $data_ref/$x $out/$x
echo "-----------------------------"
echo "SH to SH done"
echo "-----------------------------"
#
done

