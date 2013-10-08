#!/bin/sh

. `dirname $0`/include.sh
set -e

#export ECREGRID_DEBUG=1
#export ECREGRID_DUMP_TO_FILE="./regular_gaussian2grids.dat"
export ECREGRID_DUMP_NEAREST_POINTS=1

compare="grib_compare -P"

area1="90.0/-180.0/-90.0/180.0"
area2="90.0/0/-90.0/360"
area3="80.0/-40.0/20.0/60.0"
area4="60.0/-10.0/10.0/15.0"
area5="60.0/10.0/10.0/35.0"

valgrind=valgrind
valgrind="valgrind --leak-check=full"
valgrind=
minmax="grib_ls -p minimum,maximum"

ll1=0.25/0.25
ll2=1/1
ll3=0.5/0.5
sh=80
gg1=200 
gg2=400 
rot1="-30/10"

echo "******************************"
echo " Subarea extraction **********"
echo "******************************"

files="upper_air_gaussian.grib1 \
       surface_latlon.grib1     \
	   surface_gaussian_reduced.grib1"

for file in `echo $files`
do
#
x=y_$file-subarea.grib
echo ------------------------------
echo "$x to Subarea ..."
echo ------------------------------
ll=$ll2
a=$area3
$tool -i $data/$file -o $out/$x -a $a
$compare $data_ref/$x $out/$x
echo ------------------------------
echo "$x to Subarea done"
#
done

