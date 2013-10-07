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
sh=80
gg1=200
gg2=400
rot1=(-30, 10)

os.environ["ECREGRID_DUMP_NEAREST_POINTS"]="1"
os.environ["ECREGRID_DISABLE_LSM"]="1"


for infile in ["surface_latlon_reduced.grib1"]:

    print "================================================"
    outfile="y_%s-ll_subarea1.grib" % infile
    print "Test %s: %s" % (__file__, outfile)

    fd=ecregrid.get_field_description("regular_ll")

    ecregrid.set_area(fd, *area3)
    ecregrid.set_increments(fd,*ll2)

    tester.run_transformation(infile, outfile, fd)
    output=tester.compare_with_reference(outfile)

    ecregrid.field_description_destroy(fd)
    tester.report_output(output)

    print "================================================"
    outfile="y_%s-ll_subarea2.grib" % infile
    print "Test %s: %s" % (__file__, outfile)

    fd=ecregrid.get_field_description("regular_ll")

    ecregrid.set_area(fd, *area4)
    ecregrid.set_increments(fd, *ll2)

    tester.run_transformation(infile, outfile, fd)
    output=tester.compare_with_reference(outfile)
    ecregrid.field_description_destroy(fd)
    tester.report_output(output)

    print "================================================"
    outfile="y_%s-ll_subarea3.grib" % infile
    print "Test %s: %s" % (__file__, outfile)

    fd=ecregrid.get_field_description("regular_ll")

    ecregrid.set_area(fd, *area1)
    ecregrid.set_increments(fd, *ll1)

    tester.run_transformation(infile, outfile, fd)
    output=tester.compare_with_reference(outfile)
    ecregrid.field_description_destroy(fd)
    tester.report_output(output)


    print "================================================"
    outfile="y_%s-ll.grib" % infile
    print "Test %s: %s" % (__file__, outfile)

    fd=ecregrid.get_field_description("regular_ll")

    ecregrid.set_increments(fd, *ll1)

    tester.run_transformation(infile, outfile, fd)
    output=tester.compare_with_reference(outfile)
    ecregrid.field_description_destroy(fd)
    tester.report_output(output)


    print "================================================"
    outfile="y_%s-llrot_subarea1.grib" % infile
    print "Test %s: %s" % (__file__, outfile)

    fd=ecregrid.get_field_description("regular_ll")

    ecregrid.set_area(fd, *area3)
    ecregrid.set_increments(fd, *ll2)
    ecregrid.set_south_pole(fd, *rot1)
    #ecregrid.set_interpolation_method(fd, "cubic")

    tester.run_transformation(infile, outfile, fd)
    output=tester.compare_with_reference(outfile)
    ecregrid.field_description_destroy(fd)
    tester.report_output(output)


    print "================================================"
    outfile="y_%s-llrot.grib" % infile
    print "Test %s: %s" % (__file__, outfile)

    fd=ecregrid.get_field_description("regular_ll")

    ecregrid.set_increments(fd, *ll1)
    ecregrid.set_south_pole(fd, *rot1)
    #ecregrid.set_interpolation_method(fd, "cubic")

    tester.run_transformation(infile, outfile, fd)
    output=tester.compare_with_reference(outfile)
    ecregrid.field_description_destroy(fd)
    tester.report_output(output)

    print "================================================"
    outfile="y_%s-gg_subarea1.grib" % infile
    print "Test %s: %s" % (__file__, outfile)

    fd=ecregrid.get_field_description("regular_gg")

    ecregrid.set_gaussian_number(fd, gg2)
    ecregrid.set_area(fd, *area3)

    tester.run_transformation(infile, outfile, fd)
    output=tester.compare_with_reference(outfile)
    ecregrid.field_description_destroy(fd)
    tester.report_output(output)

    print "================================================"
    outfile="y_%s-gg.grib" % infile
    print "Test %s: %s" % (__file__, outfile)

    fd=ecregrid.get_field_description("regular_gg")

    ecregrid.set_gaussian_number(fd, gg2)

    tester.run_transformation(infile, outfile, fd)
    output=tester.compare_with_reference(outfile)
    ecregrid.field_description_destroy(fd)
    tester.report_output(output)

    # NB the final two tests are not currently run in tools folder
    continue

    print "================================================"
    outfile="y_%s-rg_subarea1.grib" % infile
    print "Test %s: %s" % (__file__, outfile)

    fd=ecregrid.get_field_description("reduced_gg")

    ecregrid.set_gaussian_number(fd, gg2)
    ecregrid.set_area(fd, *area3)

    tester.run_transformation(infile, outfile, fd)
    output=tester.compare_with_reference(outfile)
    ecregrid.field_description_destroy(fd)
    tester.report_output(output)


    print "================================================"
    outfile="y_%s-list.txt" % infile
    print "Test %s: %s" % (__file__, outfile)

    fd=ecregrid.get_field_description("list")
    ecregrid.set_list_of_points_file(fd, os.path.join(data_in, "proba.txt") )
    ecregrid.set_list_of_points_file_type(fd, "ascii")
    ecregrid.set_interpolation_method(fd, "bilinear")

    tester.run_transformation(infile, outfile, fd)
    output=tester.compare_with_reference(outfile)
    ecregrid.field_description_destroy(fd)
    tester.report_output(output)

