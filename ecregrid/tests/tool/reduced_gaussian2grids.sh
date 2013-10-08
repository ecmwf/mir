#!/bin/sh

. `dirname $0`/include.sh
set -e

#export ECREGRID_DUMP_TO_FILE="./reduced_gaussian2grids.dat"

#compare=grib_compare
compare="grib_compare -P"
minmax="grib_ls -p minimum,maximum"

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
sh=80
gg1=200 
gg2=400 
rot1="-30/10"

echo "******************************"
echo " Upper RG to grids  **********"
echo "******************************"

files="upper_air_gaussian_reduced.grib1"

for file in `echo $files`
do
#
echo ------------------------------
echo "RG to LL Subarea 1 ..."
echo ------------------------------
ll=$ll2
a=$area3
x=y_$file-ll_subarea1.grib
$tool -i $data/$file -o $out/$x -l $ll -a $a -m linearfit
#$compare $data_ref/$x $out/$x
$minmax $data/$file $out/$x
echo ------------------------------
echo "RG to LL Subarea 1 done"
#
#
echo ------------------------------
echo "RG to LL Subarea 2 ..."
echo ------------------------------
ll=$ll2
a=$area4
x=y_$file-ll_subarea2.grib
$tool -i $data/$file -o $out/$x -l $ll -a $a -m linearfit
$compare $data_ref/$x $out/$x
$minmax $data/$file $out/$x
echo ------------------------------
echo "RG to LL Subarea 2 done"
#
#
echo ------------------------------
echo "RG to LL Subarea 3 ..."
echo ------------------------------
ll=$ll1
a=$area1
x=y_$file-ll_subarea3.grib
$tool -i $data/$file -o $out/$x  -l $ll -a $a -m linearfit
$compare $data_ref/$x $out/$x
$minmax $data/$file $out/$x
echo ------------------------------
echo "RG to LL Subarea 3 done"
#
echo ------------------------------
echo "RG to LL ..."
echo ------------------------------
ll=$ll1
x=y_$file-ll.grib
$tool -i $data/$file -o $out/$x -l $ll -m linearfit
$compare $data_ref/$x $out/$x
$minmax $data/$file $out/$x
echo ------------------------------
echo "RG to LL done"
#
echo ------------------------------
echo "RG to cellcentred LL ..."
echo ------------------------------
ll=$ll1
x=y_$file-ll-cellcentred.grib
$tool -i $data/$file -o $out/$x -l $ll -m linearfit -y cellcentred
$compare $data_ref/$x $out/$x
$minmax $data/$file $out/$x
echo ------------------------------
echo "RG to cellcentred LL done"
#
echo ------------------------------
echo "GG to List of Points ..."
echo ------------------------------
x=y_$file-list.txt
$tool -i $data/$file -o $out/$x  -w $data/proba.txt -m linearfit
diff $data_ref/$x $out/$x
echo ------------------------------
echo "GG to List of Points done"
#
echo ------------------------------
echo "RG to Rotated LL Subarea ..."
echo ------------------------------
a=$area3
rot=$rot1
ll=$ll2
x=y_$file-llrot_subarea1.grib
$tool -i $data/$file -o $out/$x -l $ll -a $a -r $rot -m cubic
$compare $data_ref/$x $out/$x
$minmax $data/$file $out/$x
echo ------------------------------
echo "RG to Rotated LL Subarea done"
#
echo ------------------------------
echo "RG to Rotated LL ..."
echo ------------------------------
ll=$ll1
rot=$rot1
x=y_$file-llrot.grib
$tool -i $data/$file -o $out/$x -l $ll -r $rot -m cubic
$compare $data_ref/$x $out/$x
$minmax $data/$file $out/$x
echo ------------------------------
echo "RG to Rotated LL done"
#
echo ------------------------------
echo "RG to GG Subarea ..."
echo ------------------------------
a=$area3
gg=$gg2
x=y_$file-gg_subarea1.grib
$tool -i $data/$file -o $out/$x -g $gg -a $a -m linearfit
$compare $data_ref/$x $out/$x
$minmax $data/$file $out/$x
echo ------------------------------
echo "RG to GG Subarea done"
#
echo ------------------------------
echo "RG to GG ..."
echo ------------------------------
gg=$gg2
x=y_$file-gg.grib
$tool -i $data/$file -o $out/$x -g $gg -m linearfit
$compare $data_ref/$x $out/$x
$minmax $data/$file $out/$x
echo ------------------------------
echo "RG to GG done"
#
echo ------------------------------
echo "RG to RG ..."
echo ------------------------------
x=y_$file-rg.grib
$tool -i $data/$file -o $out/$x -g $gg -y reduced -m linearfit
$compare $data_ref/$x $out/$x
$minmax $data/$file $out/$x
echo ------------------------------
echo "RG to RG done"
#
done

