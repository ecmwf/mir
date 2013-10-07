#!/bin/sh

cp $1 ecregrid.grib
cp $2 emos.grib
rm -f PlotFile?
metview -b metview_macro
convert PlotFile1 x.png
xv x.png&

