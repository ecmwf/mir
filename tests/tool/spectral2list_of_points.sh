#!/bin/sh

. `dirname $0`/include.sh
set -e

#export ECREGRID_DEBUG=1
#export ECREGRID_DUMP_TO_FILE="./regular_gaussian2grids.dat"
#export ECREGRID_DUMP_NEAREST_POINTS=1

compare="diff"

valgrind=valgrind
valgrind="valgrind --leak-check=full"
valgrind=

echo "******************************"
echo " Spectral to list of points **"
echo "******************************"

files="spectral.grib"

for file in `echo $files`
do
#
x=y_$file-list.txt

#ecregrid -i $data/$file -o $out/$x -w $data/list_of_points.txt
$tool -i $data/$file -o $out/$x -w $data/list_of_points.txt

$compare $data_ref/$x $out/$x

echo ------------------------------
#
done

