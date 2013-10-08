#!/bin/sh

. `dirname $0`/include.sh
set -e

compare=grib_compare

area1="90.0/-180.0/-90.0/180.0"
area2="90.0/0/-90.0/360"
area3="80.0/-40.0/20.0/60.0"
area4="60.0/-10.0/10.0/15.0"
area5="60.0/10.0/10.0/35.0"

valgrind=valgrind
valgrind="valgrind --leak-check=full"
valgrind=

echo "******************************"
echo "Same As                   ****"
echo "******************************"

ll1=0.125/0.125
ll2=0.125/0.125
ll3=0.125/0.125
sh=80
sh1=511
gg1=640 
gg2=640 
rot1="-30/10"


echo ------------------------------
echo "LL to LL"
echo ------------------------------
file="upper_air_latlon.grib1"
ll=$ll3
x=y_$file-ll.grib
$tool -i $data/$file -o $out/$x -l $ll
$compare $data/$file $out/$x
echo ------------------------------
echo "LL to LL done"
#
#
echo ------------------------------
echo "GG to GG ..."
echo ------------------------------
file="upper_air_gaussian.grib1"
gg=$gg1
x=y_$file-gg.grib
$tool -i $data/$file -o $out/$x -g $gg
$compare $data/$file $out/$x
#
echo ------------------------------
echo "GG to GG done"
#
echo ------------------------------
echo "RG to RG ..."
echo ------------------------------
file="upper_air_gaussian_reduced.grib1"
gg=$gg1
x=y_$file-rg.grib
$tool -i $data/$file -o $out/$x -g $gg -y reduced
$compare $data/$file $out/$x
#
echo ------------------------------
echo "RG to RG done"
#

echo ------------------------------
echo "SH to SH ..."
echo ------------------------------
ll=$ll1
file="spectral_511.grib1"
x=y_$file-sh.grib
$tool -i $data/$file -o $out/$x -t $sh1 -q12
$compare $data/$file $out/$x
echo ------------------------------
echo "SH to SH done"
#
exit
