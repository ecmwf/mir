#!/bin/sh

. `dirname $0`/include.sh
set -e

#export ECREGRID_DEBUG=1
#export ECREGRID_DUMP_TO_FILE="./spectral2grids_wind.dat"

compare="grib_compare -f"

area1="90.0/-180.0/-90.0/180.0"
area2="90.0/0/-90.0/360"
area3="80.0/-40.0/20.0/60.0"
area4="60.0/-10.0/10.0/15.0"
area5="60.0/10.0/10.0/35.0"

#tool="valgrind --leak-check=full ${tool}"
#tool="valgrind --tool=callgrind --dump-instr=yes --trace-jump=yes ${tool}"

ll1=0.25/0.25
ll2=1/1
ll3=0.5/0.5
gg1=200 
gg2=400 
rot1="-30/10"

echo "******************************"
echo "SH to grids WIND          ****"
echo "******************************"

files="vortdiv.grib"

for file in `echo $files`
do
 echo "-----------------------------"
 echo " 1/6 SH to LL ... "
 echo "-----------------------------"
 x=y_$file-latlon_0.25_0.25.grib
 $tool -i $data/$file -o $out/$x -l $ll1 -z on -c on-fly
 $compare $data_ref/$x $out/$x
 echo "-----------------------------"
 echo " 1/6 SH to LL DONE"
 ##
 echo "-----------------------------"
 echo " 2/6 SH to LL Subarea ..."
 echo "-----------------------------"
 x=y_$file-latlon_0.5_0.5_subarea.grib
 $tool -i $data/$file -o $out/$x -l $ll3 -a $area4 -z on
 $compare $data_ref/$x $out/$x
 echo "-----------------------------"
 echo " 2/6 SH to LL Subarea DONE"
 ##
 echo "-----------------------------"
 echo " 3/6 SH to RG ..."
 echo "-----------------------------"
 x=y_$file-rg_400.grib
 $tool -i $data/$file -o $out/$x -g $gg2 -y reduced -z on
 $compare $data_ref/$x $out/$x
 echo "-----------------------------"
 echo " 3/6 SH to RG DONE"
 echo "-----------------------------"
 ##
 echo " 4/6 SH to GG ..."
 echo "-----------------------------"
 gg=$gg2
 x=y_$file-gg_$gg.grib
 $tool -i $data/$file -o $out/$x -g $gg -z on
 $compare $data_ref/$x $out/$x
 echo "-----------------------------"
 echo " 4/6 SH to GG DONE"
 ##
 echo "-----------------------------"
 echo " 5/6 SH to GG Subarea ..."
 echo "-----------------------------"
 gg=$gg2
 x=y_$file-gg_400_subarea.grib
 $tool -i $data/$file -o $out/$x -a $area3 -g $gg -z on
 $compare $data_ref/$x $out/$x
 echo "-----------------------------"
 echo " 5/6 SH to GG Subarea DONE"
 #
 echo "-----------------------------"
 echo " 6/6 SH to Rotated LL"
 echo "-----------------------------"
 x=y_$file-latlon_0.25_0.25_subarea_rotated.grib
 $tool -i $data/$file -o $out/$x -a $area4 -l $ll2 -r $rot1 -z on
 $compare $data_ref/$x $out/$x
 echo "-----------------------------"
 echo " 6/6 SH to Rotated LL DONE"
 echo "-----------------------------"
 #
 #===============================================
done
