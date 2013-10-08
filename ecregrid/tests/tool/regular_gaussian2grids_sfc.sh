#!/bin/sh

. `dirname $0`/include.sh
set -e
#set -x

#export ECREGRID_DEBUG=1
#export ECREGRID_DUMP_TO_FILE="./regular_gaussian2grids_sfc.dat"
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
echo " GG to grids        **********"
echo "******************************"

files="surface_gaussian.grib1"

for file in `echo $files`
do
#
echo ------------------------------
echo "GG to LL Subarea 1 ..."
echo ------------------------------
ll=$ll2
a=$area3
x=y_$file-ll_subarea1.grib
$tool -i $data/$file -o $out/$x -l $ll -a $a
echo "$tool -i $data/$file -o $out/$x -l $ll -a $a"
$compare $data_ref/$x $out/$x
echo "$compare $data_ref/$x $out/$x"
$minmax $data/$file $out/$x
echo ------------------------------
echo "GG to LL Subarea 1 done"
echo ------------------------------
echo "GG to LL ..."
echo ------------------------------
ll=$ll1
x=y_$file-ll.grib
echo "$tool -i $data/$file -o $out/$x -l $ll"
$tool -i $data/$file -o $out/$x -l $ll
echo "$compare $data_ref/$x $out/$x"
$compare $data_ref/$x $out/$x
$minmax $data/$file $out/$x
#exit 1
echo ------------------------------
echo "GG to LL done"
#
echo ------------------------------
echo "GG to List of Points ..."
echo ------------------------------
#
x=y_$file-list.txt
$tool -i $data/$file -o $out/$x  -w $data/proba.txt -m bilinear
diff $data_ref/$x $out/$x
echo ------------------------------
echo "GG to List of Points done"
#
echo ------------------------------
echo "GG to Rotated LL Subarea ..."
echo ------------------------------
#
a=$area3
rot=$rot1
ll=$ll2
x=y_$file-llrot_subarea1.grib
$tool -i $data/$file -o $out/$x -l $ll -a $a -r $rot -m cubic
$compare $data_ref/$x $out/$x
$minmax $data/$file $out/$x
echo ------------------------------
echo "GG to Rotated LL Subarea done"
#
echo ------------------------------
echo "GG to Rotated LL ..."
echo ------------------------------
ll=$ll1
rot=$rot1
x=y_$file-llrot.grib
$tool -i $data/$file -o $out/$x -l $ll -r $rot -m cubic
$compare $data_ref/$x $out/$x
$minmax $data/$file $out/$x
echo ------------------------------
echo "GG to Rotated LL done"
#
echo ------------------------------
echo "GG to GG Subarea ..."
echo ------------------------------
a=$area3
gg=$gg2
x=y_$file-gg_subarea1.grib
$tool -i $data/$file -o $out/$x -g $gg -a $a
$compare $data_ref/$x $out/$x
$minmax $data/$file $out/$x
echo ------------------------------
echo "GG to GG Subarea done"
#
#
echo ------------------------------
echo "GG to RG ..."
echo ------------------------------
x=y_$file-rg.grib
$tool -i $data/$file -o $out/$x -g $gg -y reduced
$compare $data_ref/$x $out/$x
$minmax $data/$file $out/$x
echo ------------------------------
echo "GG to RG done"
#
echo ------------------------------
echo "GG to GG ..."
echo ------------------------------
gg=$gg2
x=y_$file-gg.grib
$tool -i $data/$file -o $out/$x -g $gg
$compare $data_ref/$x $out/$x
$minmax $data/$file $out/$x
echo ------------------------------
echo "GG to GG done"
#
done

