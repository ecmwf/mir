# Copyright 2005-2007 ECMWF
# 
# Licensed under the GNU Lesser General Public License which
# incorporates the terms and conditions of version 3 of the GNU
# General Public License.
# See LICENSE and gpl-3.0.txt for details.

set -ea

echo
echo "TEST: $0"

data_dir=""

# save current working dir
save=`pwd`

#export ECREGRID_DEBUG=1

if [ -z "${data_dir}" ]
then
  cd ../..
  path=`pwd`
  #tool=${path}/tools/ecregrid
  tool=/usr/local/apps/ecregrid/1.8.4/bin/ecregrid
  data=${path}/data_in
  data_ref=${path}/data_ref
  out=data_out

  ECREGRID_SHARE_DIR=$path/share/ecregrid
  export ECREGRID_SHARE_DIR
  echo "ECREGRID_SHARE_DIR" $ECREGRID_SHARE_DIR

#  ECREGRID_DATA_DIR=$path
#  export ECREGRID_DATA_DIR
#  echo "ECREGRID_DATA_DIR" $ECREGRID_DATA_DIR
fi

compare="grib_compare -f"

valgrind=valgrind
valgrind="valgrind --leak-check=full"
valgrind=

# go back to current working dir
cd $save

set -u

