#!/bin/ksh

# Common initialisation

MARS=mars

cd ../..
path=`pwd`
data=${path}/data_in
data_ref=${path}/data_ref
out=data_out

ECREGRID_SHARE_PATH=$path/share/ecregrid
export ECREGRID_SHARE_DIR
echo "ECREGRID_SHARE_Dir" $ECREGRID_SHARE_DIR

#ECREGRID_DATA_DIR=$path/data_dir
#export ECREGRID_DATA_DIR
#echo "ECREGRID_DATA_DIR" $ECREGRID_DATA_DIR

cd - > /dev/null

set -e

mkdir -p $out
#export ECREGRID_DEBUG=1






