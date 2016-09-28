#!/usr/bin/env python

import sys

mir_tool="$<TARGET_FILE:mir-tool>"
mir_compare="$<TARGET_FILE:mir-compare>"
grib_get="$<TARGET_FILE:grib_get>"
grib_compare="$<TARGET_FILE:grib_compare>"

print sys.argv[1:]

print mir_tool
