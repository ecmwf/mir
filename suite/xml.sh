#!/usr/bin/ksh
#
# Utility script for writing XML tags
# Set output file using set_xml_output
#     and open / close / write tags as required
# Tag values can be written using PRINT function
# It is recommended that xmllint --format is used
#     to verify and style the output produced
#     by this script
#

set -A STACK


XMLOUT=""
local=""

function PRINT {        # Define a standard printing format
  #print -n -- "$1"
  if [[ $XMLOUT != "" ]]; then
    print -n -- "$1" >> $XMLOUT
  else
    extra="$1"
    local="$local$extra"

  fi
}

function flush_buffer_to_file {
  filename="$1"
  #rm -f "$filename"
  echo $local > "$filename"
  local=""
}

function set_xml_output {
   # pass in an output file to write to
   XMLOUT="$1"
   APPEND="$2"
   if [[ $XMLOUT != "" ]]; then
       if [[ $APPEND != "append" ]]; then
           rm -f "$XMLOUT"
           touch "$XMLOUT"
       fi
   fi
}


function start {         # No ARGS. Clears STACK
  unset STACK
  set -A STACK
}


function push {          # ARG: value. Pushes onto STACK
    STACK[${#STACK[*]}]=$1
}

function pop {           # No ARGS. Pops value from STACK
  if [[ ${#STACK[*]} -gt 0 ]]; then
      len=${#STACK[*]}
      (( len=len-1 ))
      unset STACK[$len]
  fi
}


function open_tag {      # ARG: tag_name optional key value pairs as single string
                         # e.g. open_tag "Detail" "arg='val' arg1='other'"
  push $1
  PRINT "<$1"
  if [[ $# -gt 1 ]]; then
    PRINT " "
    PRINT "$2"
  fi
  PRINT ">"

}

function write_close_tag { # ARG: Tag name

    PRINT "</"
    PRINT "$1"
    PRINT ">"
}

function close_tag {     # No ARGS.

  if [[ ${#STACK[*]} -gt 0 ]]; then
    PRINT "</"
    # get the last item on the stack
    lastoutput=""
    if [[ ${#STACK[*]} -gt 0 ]]; then
        len=${#STACK[*]}
        (( len=len-1 ))
        lastoutput=${STACK[$len]}
    fi
    PRINT "$lastoutput"
    PRINT ">"
    pop
  fi

}


function write_tag {     # ARGS: tag_name value
  open_tag "$*"
  close_tag 
  
}

function comment {
  PRINT "<!-- "
  PRINT "$1"
  PRINT " -->"
}

function finish {         # No ARGS. Closes all outstanding tags
  while [[ ${#STACK[*]} -gt 0 ]]; do
    close_tag
  done
}


# Example usage:
#
# start
# comment "This is a comment"
#
# open_tag "Tag"
# PRINT "This is the value of the tag"
# close_tag
#
# open_tag "NestedLevel1"
# open_tag "NestedLevel2"
# PRINT "Value"
# close_tag
# close_tag
#
# write_tag "SingleLineTagNameWithNoInnerValue" "Attr1='foo' Attr2='bar'"
#
# etc..
#
# finish




