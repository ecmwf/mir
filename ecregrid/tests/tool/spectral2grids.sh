#!/bin/sh
. `dirname $0`/include.sh
set -e

#export ECREGRID_DUMP_TO_FILE="./spectral2grids.dat"
#export ECREGRID_DEBUG=1

compare=grib_compare

area1="90.0/-180.0/-90.0/180.0"
area2="90.0/0/-90.0/360"
area3="80.0/-40.0/20.0/60.0"
area4="60.0/-10.0/10.0/15.0"
area5="60.0/10.0/10.0/35.0"

valgrind=valgrind
valgrind="valgrind --leak-check=full"
valgrind=

ll1=0.25/0.25
ll2=1/1
ll3=0.5/0.5
gg1=200 
gg2=200 
rot1="-30/10"

echo "******************************"
echo "SH to grids               ****"
echo "******************************"

files="spectral_511.grib1"

echo "Using tool $tool"

for file in `echo $files`
do
echo ------------------------------
echo "SH to LL ..."
echo ------------------------------
x=y_$file-latlon_0.25_0.25.grib
$tool -i $data/$file -o $out/$x -l $ll1 -c fileio
$compare $data_ref/$x $out/$x
echo ------------------------------
echo "SH to LL done"
#
#
echo ------------------------------
echo "SH to LL Subarea ..."
echo ------------------------------
x=y_$file-latlon_0.5_0.5_subarea.grib
$tool -i $data/$file -o $out/$x -l $ll3 -a $area4 -z on -c fileio
$compare $data_ref/$x $out/$x
echo ------------------------------
echo "SH to LL Subarea done"
#
echo ------------------------------
echo "SH to LL Subarea Frame ..."
echo ------------------------------
x=y_$file-latlon_0.5_0.5_subarea_frame_10.grib
$tool -i $data/$file -o $out/$x -l $ll3 -a $area4 -z on -c fileio -f 10
$compare $data_ref/$x $out/$x
echo ------------------------------
echo "SH to LL Subarea Frame done"
#
echo ------------------------------
echo "SH to RG ..."
echo ------------------------------
#
x=y_$file-rg_400.grib
$tool -i $data/$file -o $out/$x -g $gg2 -y reduced -z on -c fileio
$compare $data_ref/$x $out/$x
#
echo ------------------------------
echo "SH to RG done"
#
echo ------------------------------
echo "SH to GG ..."
echo ------------------------------
x=y_$file-gg_400.grib
$tool -i $data/$file -o $out/$x -g $gg2 -z on -c fileio
$compare $data_ref/$x $out/$x
echo ------------------------------
echo "SH to GG done"
#
#
echo ------------------------------
echo "SH to GG Subarea ..."
echo ------------------------------
x=y_$file-gg_400_subarea.grib
$tool -i $data/$file -o $out/$x -a $area3 -g $gg2 -z on
$compare $data_ref/$x $out/$x
echo ------------------------------
echo "SH to GG Subarea done"
#
echo ------------------------------
echo "SH to GG Subarea Frame ..."
echo ------------------------------
x=y_$file-gg_400_subarea_frame_10.grib
$tool -i $data/$file -o $out/$x -a $area3 -g $gg2 -z on -f 10
$compare $data_ref/$x $out/$x
echo ------------------------------
echo "SH to GG Subarea Frame done"
#
#
echo ------------------------------
echo "SH to Rotated LL Subarea ..."
echo ------------------------------
#
x=y_$file-latlon_0.25_0.25_subarea_rotated.grib
$tool -i $data/$file -o $out/$x -a $area4 -l $ll2 -r $rot1 -z on -c fileio
$compare $data_ref/$x $out/$x
echo ------------------------------
echo "SH to Rotated LL done"
#
#===============================================
done

