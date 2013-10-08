#!/bin/sh

. ./include.sh
set -e

# to be run on c2b

#export ECREGRID_DEBUG=1
#export ECREGRID_DISABLE_LSM=1

export ECREGRID_WRITE_CHUNKS=1
export ECREGRID_LSM_PATH=/fwsm/lb/user/nag/lsm

compare="grib_compare -P"

valgrind=valgrind
valgrind="valgrind --leak-check=full"
valgrind=
minmax="grib_ls -p minimum,maximum"

gg=400 



echo "******************************"
echo " Anisotropy         **********"
echo "******************************"

files="/fwsm/lb/user/nag/tmp/38r2/climate/7999l_2/sub_grid_diffs"

for file in `echo $files`
do
#
x=y-rg$gg-anisotropy.grib
$tool -i $file -o $out/$x -m averageweighted -g $gg -y reduced -v predefined -j anisotropy
$compare $data_ref/$x $out/$x
#$minmax $data/$file $out/$x
echo ------------------------------
#
done

