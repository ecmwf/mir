#!/usr/local/bin/python
import sys
sys.path.append("../../python")
import ecregrid
import tester

for infile in ["spectral_511.grib1"]:

    print "================================================"
    outfile="y_%s-sh_319.grib" % infile
    print "Test %s: %s" % (__file__, outfile)

    fd=ecregrid.get_field_description("sh")

    ecregrid.set_truncation(fd, 319)

    tester.run_transformation(infile, outfile, fd)
    output=tester.compare_with_reference(outfile)

    ecregrid.field_description_destroy(fd)
    tester.report_output(output)





