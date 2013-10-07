#!/bin/sh

. ./include.sh
set -e

#export ECREGRID_DEBUG=1
export ECREGRID_DISABLE_LSM=1

data=../data_in

data_ref=../data
data_ref=.

out=.

compare=grib_compare
tool=../tools/ecregrid

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

banner "be back"
exit
echo "******************************"
echo "List of points to grids ******"
echo "******************************"

files="upper_air_gaussian.txt"

for file in `echo $files`
do
#===============================================
# to LL
#---------
#
#   Global to Subarea
#
ll=$ll2
a=$area3
x=y_$file-ll_subarea1.grib
$tool -i $data/$file -x ascii -o $out/$x -l $ll -a $a
$compare $data_ref/$x $out/$x
#
#
ll=$ll2
a=$area4
x=y_$file-ll_subarea2.grib
$tool -i $data/$file -x ascii -o $out/$x -l $ll -a $a
$compare $data_ref/$x $out/$x
#
#
ll=$ll1
a=$area1
x=y_$file-ll_subarea3.grib
$tool -i $data/$file -x ascii -o $out/$x  -l $ll -a $a
$compare $data_ref/$x $out/$x
#
#
#   Global to Global
#
ll=$ll1
x=y_$file-ll.grib
$tool -i $data/$file -x ascii -o $out/$x -l $ll
$compare $data_ref/$x $out/$x
#
#===============================================
# to List of Points
#---------
# be back
#
if ( test $TON -eq 1) ; then
x=y_$file-list.txt
$tool -i $data/$file -x ascii -o $out/$x  -w $data/proba.txt -m bilinear
diff $data_ref/$x $out/$x
fi
#
#
#===============================================
# to Rotated Regular LL
#---------
#   Global to Subarea
#
a=$area3
rot=$rot1
ll=$ll2
x=y_$file-llrot_subarea1.grib
$tool -i $data/$file -x ascii -o $out/$x -l $ll -a $a -r $rot
$compare $data_ref/$x $out/$x
#
#
#   Global to Global
#
ll=$ll1
rot=$rot1
x=y_$file-llrot.grib
$tool -i $data/$file -x ascii -o $out/$x -l $ll -r $rot
$compare $data_ref/$x $out/$x
#
#
#===============================================
# to Regular Gaussian
#---------
#   Global to Subarea
#
a=$area3
gg=$gg2
x=y_$file-gg_subarea1.grib
$tool -i $data/$file -x ascii -o $out/$x -g $gg -a $a
$compare $data_ref/$x $out/$x
#
#
#   Global to Global
#
gg=$gg2
x=y_$file-gg.grib
$tool -i $data/$file -x ascii -o $out/$x -g $gg
$compare $data_ref/$x $out/$x
#
#
#===============================================
# to Reduced Gaussian
#---------
#   Global to Subarea
#
a=$area3
gg=$gg2
x=y_$file-rg_subarea1.grib
$tool -i $data/$file -x ascii -o $out/$x -g $gg -a $a -y
$compare $data_ref/$x $out/$x
#
#
#   Global to Global
#
x=y_$file-rg.grib
$tool -i $data/$file -x ascii -o $out/$x -g $gg -y
$compare $data_ref/$x $out/$x
#
#
#===============================================
echo "********* missing:  list of points"
banner "be back"
done

