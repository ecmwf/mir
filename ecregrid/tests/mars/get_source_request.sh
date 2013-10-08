#!/bin/ksh

# for given grib file,
# we build the mars request from the grib_ls -m keywords 

grib_file=$1
request_file=$2

tmp_file=".tmp"
output=`grib_ls -m $grib_file > $tmp_file`
count=0
set -A keys
set -A vals

while read line
do
  set -A words $line
  if [[ $count -gt 0 ]]; then
      if [[ ${#keys[*]} -eq 0 ]]; then
        set -A keys $line
      else
          if [[ ${#vals[*]} -eq 0 ]]; then
             set -A vals $line
          fi
      fi
  fi
  (( count = count + 1 ))
done < `echo $tmp_file`

rm -f $tmp_file


echo "read," > $request_file
echo "    source='$grib_file'," >> $request_file
count=0
while [[ $count -lt ${#keys[*]} ]]; do
    echo "    ${keys[$count]}='${vals[$count]}'," >> $request_file
    (( count=count+1 ))
done
