#!/usr/local/bin/python
import os, sys
sys.path.append("../../python")
import ecregrid
import tester
from locations import data_in

for infile in ["upper_air_gaussian.grib1"]:

    print "================================================"
    outfile="y_upper_air_gaussian.grib1-ll_subarea1.grib"
    print "Test %s: %s" % (__file__, outfile)

    fd=ecregrid.get_field_description("regular_ll")

    ecregrid.set_area(fd, 80, -40, 20, 60)
    ecregrid.set_increments(fd, 1, 1)

    tester.run_transformation(infile, outfile, fd)
    output=tester.compare_with_reference(outfile)
    ecregrid.field_description_destroy(fd)
    tester.report_output(output)


    print "================================================"
    outfile="y_upper_air_gaussian.grib1-ll_subarea2.grib"
    print "Test %s: %s" % (__file__, outfile)

    fd=ecregrid.get_field_description("regular_ll")

    ecregrid.set_area(fd, 60, -10, 10, 15)
    ecregrid.set_increments(fd, 1, 1)

    tester.run_transformation(infile, outfile, fd)
    output=tester.compare_with_reference(outfile)
    ecregrid.field_description_destroy(fd)
    tester.report_output(output)


    print "================================================"
    outfile="y_upper_air_gaussian.grib1-ll_subarea3.grib"
    print "Test %s: %s" % (__file__, outfile)

    fd=ecregrid.get_field_description("regular_ll")

    ecregrid.set_area(fd, 90, -180, -90, 180)
    ecregrid.set_increments(fd, 0.25, 0.25)

    tester.run_transformation(infile, outfile, fd)
    output=tester.compare_with_reference(outfile)
    ecregrid.field_description_destroy(fd)
    tester.report_output(output)


    print "================================================"
    outfile="y_upper_air_gaussian.grib1-ll.grib"
    print "Test %s: %s" % (__file__, outfile)

    fd=ecregrid.get_field_description("regular_ll")

    ecregrid.set_increments(fd, 0.25, 0.25)

    tester.run_transformation(infile, outfile, fd)
    output=tester.compare_with_reference(outfile)
    ecregrid.field_description_destroy(fd)
    tester.report_output(output)


    print "================================================"
    outfile="y_upper_air_gaussian.grib1-list.txt"
    print "Test %s: %s" % (__file__, outfile)

    fd=ecregrid.get_field_description("list")

    ecregrid.set_list_of_points_file(fd, os.path.join(data_in, "proba.txt") )
    ecregrid.set_list_of_points_file_type(fd, "ascii")
    ecregrid.set_interpolation_method(fd, "bilinear")

    tester.run_transformation(infile, outfile, fd)
    output=tester.compare_with_reference(outfile)
    ecregrid.field_description_destroy(fd)
    tester.report_output(output)


    print "================================================"
    outfile="y_upper_air_gaussian.grib1-llrot_subarea1.grib"
    print "Test %s: %s" % (__file__, outfile)

    fd=ecregrid.get_field_description("regular_ll")

    ecregrid.set_increments(fd, 1, 1)
    ecregrid.set_area(fd, 80, -40, 20, 60)
    ecregrid.set_south_pole(fd, -30, 10)
    ecregrid.set_interpolation_method(fd, "cubic")

    tester.run_transformation(infile, outfile, fd)
    output=tester.compare_with_reference(outfile)
    ecregrid.field_description_destroy(fd)
    tester.report_output(output)
