#!/usr/bin/ksh
set +x

function full_name {           # ARG: grid_shortname
  
  case $1 in
    "red_gg") echo "Reduced Gaussian";;
    "reg_gg") echo "Regular Gaussian";;
    "red_ll") echo "Reduced Latlon";;
    "reg_ll") echo "Regular Latlon";;
    "rot_red_gg") echo "Rotated Reduced Gaussian";;
    "rot_reg_gg") echo "Rotated Regular Gaussian";;
    "rot_red_ll") echo "Rotated Reduced Latlon";;
    "rot_reg_ll") echo "Rotated Regular Latlon";;
    "sh") echo "Spherical Harmonics";;
    "polar") echo "Polar Stereographic";;
    "uns") echo "Unstructured";;
    *) echo "Undefined";;
  esac


}


function get_test_temp_filename {

    echo "test_${1}.xml"
}

function relpath {
   # returns path relative to root folder for this test
   relative_path $1 $output_root
}

function COPY {
    # copy and leave in a state where we can modify
    cp -f $1 $2
    chmod 755 $2
}

function get_version_from_mars_output {
    # examines output from mars request and gets the 
    # version of the mars client 
    # usage: $1 = interpolator name 
    #        $2 = mars output file dump

    str=`cat $2 | grep -i $1 | grep "INFO" | grep -i version`
    # get the final word of this line using array
    set -A array $str
    if [[ ${#array[*]} -gt 0 ]]; then
       len=${#array[*]}
       (( len=len-1 ))
       echo ${array[$len]}
    else
       echo "Unknown"
    fi

}

function get_peak_from_massif_output {
    # args <massif_output_file_path> 
    # reads a massif output file and extracts peak memory usage
    # returns peak mem usage in bytes
    # the sum of the parameters mem_heap and mem_heap_extra

    if [[ $# -lt 1 ]]; then
        echo "Usage massif_peak.sh MASSIF_OUTPUT_FILE"
        exit 1
    fi

    while read line
    do
        if [[ `echo "$line" | grep "snapshot"` != "" ]]; then
            found=0
            mem=""
            while [[ $found -eq 0 ]]; do
                read l
                teststr=`echo $l | grep "mem_heap"`
                if [[ $teststr != "" ]]; then
                    (( mem+=$teststr ))
                fi
                if [[ `echo $l | grep "heap_tree"` != "" ]]; then
                    if [[ `echo $l | grep "peak"` != "" ]]; then
                        echo $mem
                        return 0
                     fi
                     break
                fi
            done
          fi
    done < "$1"

    # If we make it here, no peak memory was found
    echo "Unknown"
    return 0

}


function timed_call_ms {

    # returns time in ms for the command passed as arg
    start_ns=`date +%s%N`
    ksh "$1" > /dev/null
    end_ns=`date +%s%N`

    (( time_ms=(end_ns-start_ns)/1000000 ))
    echo $time_ms

}



function relative_path {
  # both $1 and $2 are absolute paths
  # returns $2 relative to $1

  source=$2
  target=$1

  common_part=$source
  back=
  while [ "${target#$common_part}" = "${target}" ]; do
    common_part=$(dirname $common_part)
    back="../${back}"
  done

  echo ${back}${target#$common_part/}

}



function xpath_values {

  # takes an xml filename and xpath-style command
  # and outputs all relative values on a single line
  # NB intended for retrieval of single values only. will
  # not work well where large sets of xml data is requested
  # using the xpath command

  if [[ $# -ne 2 ]]; then
    echo "Function: xpath_values FILENAME XPATH_COMMAND"
    return 1
  fi
  filename=$1
  command=$2

  retval=""
  set -A xpath_values_array `xpath $filename $command 2>/dev/null`
  for val in ${xpath_values_array[@]}; do
    thisval=`echo $val | sed 's/^.*"\(.*\)".*$/\1/'`
    retval="$retval $thisval"
  done
  
 echo $retval
}

function as_table_row {

    set -A table_values
    table_values=$*
    
    table_str=""
		if [[ ${#table_values[*]} -gt 0 ]]; then

	    table_str=$table_str"<tr>"
		    vc=0
        last_element=""
		    for v in ${table_values[*]}; do	        
			    element=$v
			    if [[ $vc -eq 0 ]]; then
			       element="<a id='$v' target=newtab href='http://software.ecmwf.int/interpolation-testing/search?id=$v'>link to test detail</a>"
          fi	
			    table_str+="<td>${element}</td>"
     			((vc+=1))
          last_element=$v
		    done
        # add a link
        #table_str+="<a id='$last_element' target='_blank' href='http://software.ecmwf.int/interpolation-testing/search?id=$last_element'>link to test detail</a>"
		    table_str+="</tr>"
	  fi
    echo $table_str

}
