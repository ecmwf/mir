#!/bin/sh

diff -y --suppress-common-lines $1 $2 > diff.txt

./compare_dump.pl diff.txt

