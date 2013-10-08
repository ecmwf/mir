#!/bin/sh

. `dirname $0`/include.sh
set -e
#set -x

#export ECREGRID_DEBUG=1
#export ECREGRID_DUMP_TO_FILE="./reduced_latlon2grids.dat"
export ECREGRID_DUMP_NEAREST_POINTS=1

export ECREGRID_DISABLE_LSM=1

#compare=grib_compare
compare="grib_compare -P"
compare="grib_compare -P -T 2"

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
echo " Reduced LL to grids *********"
echo "******************************"
files="surface_latlon_reduced.grib1"

for file in `echo $files`
do
echo ------------------------------
echo "RL to LL Subarea 1 ..."
echo ------------------------------
ll=$ll2
a=$area3
x=y_$file-ll_subarea1.grib
echo "$tool -i $data/$file -o $out/$x -l $ll -a $a"
$tool -i $data/$file -o $out/$x -l $ll -a $a
$compare $data_ref/$x $out/$x
$minmax $data/$file $out/$x
echo ------------------------------
echo "RL to LL Subarea 1 done"
#
#
echo ------------------------------
echo "RL to LL Subarea 2 ..."
echo ------------------------------
ll=$ll2
a=$area4
x=y_$file-ll_subarea2.grib
echo "$tool -i $data/$file -o $out/$x -l $ll -a $a"
$tool -i $data/$file -o $out/$x -l $ll -a $a
$compare $data_ref/$x $out/$x
$minmax $data/$file $out/$x
echo ------------------------------
echo "RL to LL Subarea 2 done"
#
#
ll=$ll1
a=$area1
x=y_$file-ll_subarea3.grib
echo "$tool -i $data/$file -o $out/$x  -l $ll -a $a"
$tool -i $data/$file -o $out/$x  -l $ll -a $a
$compare $data_ref/$x $out/$x
$minmax $data/$file $out/$x
#
#
#   Global to Global
#
echo ------------------------------
echo "RL to LL ..."
echo ------------------------------
ll=$ll1
x=y_$file-ll.grib
echo "$tool -i $data/$file -o $out/$x -l $ll"
$tool -i $data/$file -o $out/$x -l $ll
$compare $data_ref/$x $out/$x
$minmax $data/$file $out/$x
echo ------------------------------
echo "RL to LL done"
#
echo ------------------------------
echo "RL to Rotated LL Subarea ..."
echo ------------------------------
a=$area3
rot=$rot1
ll=$ll2
x=y_$file-llrot_subarea1.grib
echo "$tool -i $data/$file -o $out/$x -l $ll -a $a -r $rot"
$tool -i $data/$file -o $out/$x -l $ll -a $a -r $rot
$compare $data_ref/$x $out/$x
$minmax $data/$file $out/$x
#
echo ------------------------------
echo "RL to Rotated LL Subarea done"
#
echo ------------------------------
echo "RL to Rotated LL ..."
echo ------------------------------
ll=$ll1
rot=$rot1
x=y_$file-llrot.grib
echo "$tool -i $data/$file -o $out/$x -l $ll -r $rot"
$tool -i $data/$file -o $out/$x -l $ll -r $rot
$compare $data_ref/$x $out/$x
$minmax $data/$file $out/$x
echo ------------------------------
echo "RL to Rotated LL done"
#
#
echo ------------------------------
echo "RL to GG Subarea ..."
echo ------------------------------
#
a=$area3
gg=$gg2
x=y_$file-gg_subarea1.grib
echo "$tool -i $data/$file -o $out/$x -g $gg -a $a"
$tool -i $data/$file -o $out/$x -g $gg -a $a
$compare $data_ref/$x $out/$x
$minmax $data/$file $out/$x
#
echo ------------------------------
echo "RL to GG Subarea done"
#
echo ------------------------------
echo "RL to GG ..."
echo ------------------------------
gg=$gg2
x=y_$file-gg.grib
echo "$tool -i $data/$file -o $out/$x -g $gg"
$tool -i $data/$file -o $out/$x -g $gg
$compare $data_ref/$x $out/$x
$minmax $data/$file $out/$x
echo ------------------------------
echo "RL to GG done"
#
exit
# ssp to rg does not work becaause of grib_api
echo ------------------------------
echo "RL to RG ..."
echo ------------------------------
a=$area3
gg=$gg2
x=y_$file-rg_subarea1.grib
echo "$tool -i $data/$file -o $out/$x -g $gg -y reduced"
$tool -i $data/$file -o $out/$x -g $gg -y reduced
$compare $data_ref/$x $out/$x
$minmax $data/$file $out/$x
echo ------------------------------
echo "RL to RG done"
#
echo ------------------------------
echo "RL to List of Points ..."
echo ------------------------------
#
x=y_$file-list.txt
echo "$tool -i $data/$file -o $out/$x  -w $data/proba.txt -m bilinear"
$tool -i $data/$file -o $out/$x  -w $data/proba.txt -m bilinear
echo "diff $data_ref/$x $out/$x"
diff $data_ref/$x $out/$x
echo ------------------------------
echo "RL to List of Points done"
echo ------------------------------
#
done

