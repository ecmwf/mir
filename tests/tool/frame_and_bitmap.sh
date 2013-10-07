#!/bin/sh

. `dirname $0`/include.sh
set -e
#set -x

#export ECREGRID_DUMP_TO_FILE="./frame_and_bitmap.dat"
#export ECREGRID_DISABLE_LSM=1

compare="grib_compare -P -T 2"

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
gg=200 

files="surface_gaussian.grib1  \
       surface_latlon.grib1"

echo "******************************"
echo "Frame and Bitmap **********"
echo "******************************"

for file in `echo $files`
do
#
echo ------------------------------
echo "GG Frame ..."
echo ------------------------------
x="y_$file-gaussian_frame.grib"
$tool -i $data/$file -o $out/$x -f 2
$compare $data_ref/$x $out/$x
echo ------------------------------
echo "GG Frame done"
#
echo ------------------------------
echo "LL Frame Inter ..."
echo ------------------------------
x=y_$file-latlon_frame_inter.grib
$tool -i $data/$file -o $out/$x -l $ll3 -f 2
$compare $data_ref/$x $out/$x
echo ------------------------------
echo "LL Frame  Inter done"
#
echo ------------------------------
echo "LL Frame ..."
echo ------------------------------
x=y_$file-latlon_frame.grib
$tool -i $data/$file -o $out/$x -f 2
$compare $data_ref/$x $out/$x
echo ------------------------------
echo "LL Frame done"
#
echo ------------------------------
echo "LL Subarea Frame ..."
echo ------------------------------
x=y_$file-latlon_subarea_frame.grib
$tool -i $data/$file -o $out/$x -a $area4 -l $ll2 -f 3
$compare $data_ref/$x $out/$x
echo ------------------------------
echo "LL Subarea Frame done"
#
#
echo ------------------------------
echo "LL Bitmap ..."
echo ------------------------------
x=y_$file-latlon_bitmap.grib
$tool -i $data/$file -o $out/$x -l $ll2 -b $data/ecm_001.bitmap
$compare $data_ref/$x $out/$x
echo ------------------------------
echo "LL Bitmap done"
#
done

