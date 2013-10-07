#!/usr/local/bin/python
import os, sys
sys.path.append("../../python")
import ecregrid
import tester
from locations import data_in
import environment


area1=(90.0, -180.0, -90.0, 180.0)
area2=(90.0, 0, -90.0, 360)
area3=(80.0, -40.0, 20.0, 60.0)
area4=(60.0, -10.0, 10.0, 15.0)
area5=(60.0, 10.0, 10.0, 35.0)

ll1=(0.25, 0.25)
ll2=(1.0, 1.0)
ll3=(0.5, 0.5)
sh=80
gg1=200
gg2=400
rot1=(-30, 10)


infile = "surface_gaussian_reduced.grib1"


print "================================================"
outfile="y_%s-ll_subarea1.grib" % infile
print "Test %s: %s" % (__file__, outfile)

fd0=ecregrid.get_field_description("regular_ll")

ecregrid.set_area(fd0, *area3)
ecregrid.set_increments(fd0,*ll2)
ecregrid.set_interpolation_method(fd0, "linear-fit")

tester.run_transformation(infile, outfile, fd0)
output=tester.compare_with_reference(outfile)

ecregrid.field_description_destroy(fd0)
tester.report_output(output)



print "================================================"
outfile="y_%s-ll_subarea2.grib" % infile
print "Test %s: %s" % (__file__, outfile)

fd1=ecregrid.get_field_description("regular_ll")

ecregrid.set_area(fd1, *area4)
ecregrid.set_increments(fd1, *ll2)
ecregrid.set_interpolation_method(fd1, "linear-fit")

tester.run_transformation(infile, outfile, fd1)
output=tester.compare_with_reference(outfile)
ecregrid.field_description_destroy(fd1)
tester.report_output(output)

