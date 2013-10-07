#!/usr/local/bin/python
import os, sys
sys.path.append("../../python")
import ecregrid
import tester
from locations import data_in


area1=(90.0, -180.0, -90.0, 180.0)
area2=(90.0, 0, -90.0, 360)
area3=(80.0, -40.0, 20.0, 60.0)
area4=(60.0, -10.0, 10.0, 15.0)
area5=(60.0, 10.0, 10.0, 35.0)

ll1=(0.25, 0.25)
ll2=(1, 1)
ll3=(0.5, 0.5)
gg1=200
gg2=400
rot1=(-30, 10)


for infile in ["vortdiv.grib"]:

    print "================================================"
    outfile="y_%s-latlon_0.25_0.25.grib" % infile
    print "Test %s: %s" % (__file__, outfile)

    fd=ecregrid.get_field_description("regular_ll")

    ecregrid.set_increments(fd,*ll1)
    ecregrid.set_auresol(fd, "on")

    tester.run_transformation(infile, outfile, fd, is_wind=True)
    output=tester.compare_with_reference(outfile)

    ecregrid.field_description_destroy(fd)
    tester.report_output(output)

    print "================================================"
    outfile="y_%s-latlon_0.5_0.5_subarea.grib" % infile
    print "Test %s: %s" % (__file__, outfile)

    fd=ecregrid.get_field_description("regular_ll")

    ecregrid.set_increments(fd,*ll3)
    ecregrid.set_area(fd, *area4)
    ecregrid.set_auresol(fd, "on")

    tester.run_transformation(infile, outfile, fd, is_wind=True)
    output=tester.compare_with_reference(outfile)

    ecregrid.field_description_destroy(fd)
    tester.report_output(output)

    print "================================================"
    outfile="y_%s-rg_400.grib" % infile
    print "Test %s: %s" % (__file__, outfile)

    fd=ecregrid.get_field_description("reduced_gg")

    ecregrid.set_gaussian_number(fd, gg2)
    ecregrid.set_auresol(fd, "on")

    tester.run_transformation(infile, outfile, fd, is_wind=True)
    output=tester.compare_with_reference(outfile)

    ecregrid.field_description_destroy(fd)
    tester.report_output(output)


    print "================================================"
    outfile="y_%s-gg_%d.grib" % (infile, gg2)
    print "Test %s: %s" % (__file__, outfile)

    fd=ecregrid.get_field_description("regular_gg")

    ecregrid.set_gaussian_number(fd, gg2)
    ecregrid.set_auresol(fd, "on")

    tester.run_transformation(infile, outfile, fd, is_wind=True)
    output=tester.compare_with_reference(outfile)

    ecregrid.field_description_destroy(fd)
    tester.report_output(output)

    print "================================================"
    outfile="y_%s-gg_%d_subarea.grib" % (infile, gg2)
    print "Test %s: %s" % (__file__, outfile)

    fd=ecregrid.get_field_description("regular_gg")

    ecregrid.set_gaussian_number(fd, gg2)
    ecregrid.set_area(fd, *area3)
    ecregrid.set_auresol(fd, "on")

    tester.run_transformation(infile, outfile, fd, is_wind=True)
    output=tester.compare_with_reference(outfile)

    ecregrid.field_description_destroy(fd)
    tester.report_output(output)

    print "================================================"
    outfile="y_%s-latlon_0.25_0.25_subarea_rotated.grib" % infile                                                    
    print "Test %s: %s" % (__file__, outfile)

    fd=ecregrid.get_field_description("rotated_ll")

    ecregrid.set_increments(fd, *ll2)
    ecregrid.set_area(fd, *area4)
    ecregrid.set_south_pole(fd, *rot1)
    ecregrid.set_auresol(fd, "on")

    tester.run_transformation(infile, outfile, fd, is_wind=True)
    output=tester.compare_with_reference(outfile)

    ecregrid.field_description_destroy(fd)
    tester.report_output(output)
