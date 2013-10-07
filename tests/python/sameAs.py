#!/usr/local/bin/python
import os, sys
sys.path.append("../../python")
import ecregrid
import tester
from locations import data_in, data_out


area1=(90.0, -180.0, -90.0, 180.0)
area2=(90.0, 0, -90.0, 360)
area3=(80.0, -40.0, 20.0, 60.0)
area4=(60.0, -10.0, 10.0, 15.0)
area5=(60.0, 10.0, 10.0, 35.0)

ll1=(0.125, 0.125)
ll2=(0.125, 0.125)
ll3=(0.125, 0.125)
sh=80
sh1=511
gg1=640
gg2=640
rot1=(-30, 10)

infile="upper_air_latlon.grib1"
print "================================================"
outfile="y_%s-ll.grib" % infile
print "Test %s: %s" % (__file__, outfile)

fd=ecregrid.get_field_description("regular_ll")

ecregrid.set_increments(fd, *ll3)

tester.run_transformation(infile, outfile, fd)
output=tester.compare_files(os.path.join(data_in, infile), os.path.join(data_out, outfile))
ecregrid.field_description_destroy(fd)
tester.report_output(output)


infile="upper_air_gaussian.grib1"
print "================================================"
outfile="y_%s-gg.grib" % infile
print "Test %s: %s" % (__file__, outfile)

fd=ecregrid.get_field_description("regular_gg")

ecregrid.set_gaussian_number(fd, gg1)

tester.run_transformation(infile, outfile, fd)
output=tester.compare_files(os.path.join(data_in, infile), os.path.join(data_out, outfile))
ecregrid.field_description_destroy(fd)
tester.report_output(output)

infile="upper_air_gaussian_reduced.grib1"
print "================================================"
outfile="y_%s-rg.grib" % infile
print "Test %s: %s" % (__file__, outfile)

fd=ecregrid.get_field_description("reduced_gg")

ecregrid.set_gaussian_number(fd, gg1)

tester.run_transformation(infile, outfile, fd)
output=tester.compare_files(os.path.join(data_in, infile), os.path.join(data_out, outfile))
ecregrid.field_description_destroy(fd)
tester.report_output(output)


infile="spectral_511.grib1"
print "================================================"
outfile="y_%s-sh_511.grib" % infile
print "Test %s: %s" % (__file__, outfile)

fd=ecregrid.get_field_description("sh")

ecregrid.set_truncation(fd, sh1)

tester.run_transformation(infile, outfile, fd)
output=tester.compare_files(os.path.join(data_in, infile), os.path.join(data_out, outfile))
ecregrid.field_description_destroy(fd)
tester.report_output(output)
