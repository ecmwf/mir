#!/bin/sh

. `dirname $0`/include.sh
set -e

compare="grib_compare -P"

valgrind=valgrind
valgrind="valgrind --leak-check=full"
valgrind=
minmax="grib_ls -p minimum,maximum"

gg1=256 
gg2=512 

file="test_reg_gg.grib"

for g in $gg1 $gg2
do
#
x=y_${file}_to_reduced_${g}.grib

echo ------------------------------
echo "$x to reduced Gaussian $g"
echo ------------------------------
$tool -i $data/$file -o $out/$x -y reduced -v 10min -g $g
$compare $data_ref/$x $out/$x
echo ------------------------------
echo "$x to reduced Gaussian $g done"
#
done

