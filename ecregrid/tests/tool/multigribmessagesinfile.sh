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
gg2=400 
rot1="-30/10"

echo "******************************"
echo "Multi Messages in File    ****"
echo "******************************"

file1="t2m.grib"
file2="spectral.grib"

echo ------------------------------
echo "RG to LL ..."
echo ------------------------------
x=y_$file1-ll_multi.grib
$tool -i $data/$file1 -o $out/$x -l $ll1
$compare $data_ref/$x $out/$x
echo ------------------------------
echo "RG to LL done"
echo ------------------------------
echo ------------------------------
echo "RG to GG ..."
echo ------------------------------
x=y_$file1-gg_400_multi.grib
$tool -i $data/$file1 -o $out/$x -g $gg2
$compare $data_ref/$x $out/$x
echo ------------------------------
echo "RG to GG done"
echo ------------------------------
echo "SH to LL ..."
echo ------------------------------
x=y_$file2-latlon_0.25_0.25_multi.grib
$tool -i $data/$file2 -o $out/$x -l $ll1 -c fileio
$compare $data_ref/$x $out/$x
echo ------------------------------
echo "SH to LL done"
#
#
echo ------------------------------
echo "SH to RG ..."
echo ------------------------------
#
x=y_$file2-rg_400_multi.grib
$tool -i $data/$file2 -o $out/$x -g $gg2 -y reduced -z on -c fileio
$compare $data_ref/$x $out/$x
#
echo ------------------------------
echo "SH to RG done"
echo ------------------------------
#
#===============================================

