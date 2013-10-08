#!/bin/sh

. `dirname $0`/include.sh
set -e

#export ECREGRID_DEBUG=1
#export ECREGRID_DUMP_TO_FILE="./grid2grids_wind.dat"

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
gg1=200 
gg2=400 
rot1="-30/10"

echo "******************************"
echo "GRID to grids WIND          ****"
echo "******************************"

files="10u10v.grib"

for file in `echo $files`
do
echo "-----------------------------"
echo "GRID to LL ... "
echo "-----------------------------"
x=y_$file-latlon_0.25_0.25.grib
$tool -i $data/$file -o $out/$x -l $ll1
$compare $data_ref/$x $out/$x
$minmax $data/$file $out/$x
echo "-----------------------------"
echo "GRID to LL DONE"
##
echo "-----------------------------"
echo "GRID to LL Subarea ..."
echo "-----------------------------"
x=y_$file-latlon_0.5_0.5_subarea.grib
$tool -i $data/$file -o $out/$x -l $ll3 -a $area4
$compare $data_ref/$x $out/$x
$minmax $data/$file $out/$x
echo "-----------------------------"
echo "GRID to LL Subarea DONE"
##
#
echo "-----------------------------"
echo "GRID to Rotated LL"
echo "-----------------------------"
#
echo "GRID to Rotated LL ..."
x=y_$file-latlon_0.25_0.25_subarea_rotated.grib
$tool -i $data/$file -o $out/$x -a $area4 -l $ll2 -r $rot1 -m cubic
$compare $data_ref/$x $out/$x
$minmax $data/$file $out/$x
echo "-----------------------------"
echo "GRID to Rotated LL DONE"
echo "-----------------------------"
#
export ECREGRID_ROUND_FOR_RG=1
echo "-----------------------------"
echo "GRID to RG ..."
echo "-----------------------------"
x=y_$file-rg_400.grib
$tool -i $data/$file -o $out/$x -g $gg2 -y reduced
$compare $data_ref/$x $out/$x
$minmax $data/$file $out/$x
echo "-----------------------------"
echo "GRID to RG DONE"
echo "-----------------------------"
##
echo "GRID to GG ..."
echo "-----------------------------"
gg=$gg2
x=y_$file-gg_$gg.grib
$tool -i $data/$file -o $out/$x -g $gg
$compare $data_ref/$x $out/$x
$minmax $data/$file $out/$x
echo "-----------------------------"
echo "GRID to GG DONE"
##
echo "-----------------------------"
echo "GRID to GG Subarea ..."
echo "-----------------------------"
gg=$gg2
x=y_$file-gg_400_subarea.grib
$tool -i $data/$file -o $out/$x -a $area3 -g $gg
$compare $data_ref/$x $out/$x
$minmax $data/$file $out/$x
echo "-----------------------------"
echo "GRID to GG Subarea DONE"
#===============================================
done

