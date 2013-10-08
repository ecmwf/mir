#!/bin/sh

. `dirname $0`/include.sh
set -e

#export ECREGRID_DUMP_TO_FILE="./average_weighted.dat"
export ECREGRID_CHECK_CONSERVING=1
#export ECREGRID_DEBUG=1

#compare=grib_compare
#compare="ls -l"
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

ll2=0.25/0.25
ll1=1/1
ll3=0.5/0.5
sh=80
gg2=200 
gg1=400 
rot1="-30/10"

echo "******************************"
echo " Average Weighted   **********"
echo "******************************"
# surface_latlon_reduced.grib1 temporary out

files="surface_gaussian_reduced.grib1 upper_air_gaussian_reduced.grib1 surface_gaussian.grib1 upper_air_gaussian.grib1 upper_air_latlon.grib1 surface_latlon.grib1"

method="averageweighted"


for file in `echo $files`
do
#
echo ------------------------------
echo "$file to LL ..."
echo ------------------------------
ll=$ll1
x=y_$file-ll_aw.grib
$tool -i $data/$file -o $out/$x -l $ll -m $method
$compare $data_ref/$x $out/$x
$minmax $data/$file 
$minmax $out/$x
echo ------------------------------
echo "$file to LL done"
#
echo ------------------------------
echo "$file to Rotated LL ..."
echo ------------------------------
ll=$ll1
rot=$rot1
x=y_$file-llrot_aw.grib
$tool -i $data/$file -o $out/$x -l $ll -r $rot -m $method
$compare $data_ref/$x $out/$x
$minmax $data/$file $out/$x
echo ------------------------------
echo "$file to Rotated LL done"
#
echo ------------------------------
echo "$file to GG ..."
echo ------------------------------
gg=$gg2
x=y_$file-gg_aw.grib
$tool -i $data/$file -o $out/$x -g $gg -m $method
$compare $data_ref/$x $out/$x
$minmax $data/$file $out/$x
echo ------------------------------
echo "$file to GG done"
#
echo ------------------------------
echo "$file to RG ..."
echo ------------------------------
x=y_$file-rg_aw.grib
gg=$gg1
$tool -i $data/$file -o $out/$x -g $gg -y reduced -m $method
$compare $data_ref/$x $out/$x
$minmax $data/$file $out/$x
echo ------------------------------
echo "$file to RG done"
#
done

