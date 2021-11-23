#!/usr/bin/env python3
#
# (C) Copyright 1996- ECMWF.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
#
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation nor
# does it submit to any jurisdiction.


import argparse
import re
import mir


class _Regex(object):
    def __init__(self, pattern):
        self._pattern = re.compile(pattern)

    def __call__(self, value):
        if not self._pattern.match(value):
            raise argparse.ArgumentTypeError(
                "must match '{}'".format(self._pattern.pattern)
            )
        return value


g = r"[ONF][1-9][0-9]*"
f = r"([0-9]*[.])?[0-9]+"

_grid = r"^" + f + r"/" + f + "|" + g + r"$"
_area = r"^-?" + f + r"/-?" + f + r"/-?" + f + r"/-?" + f + r"$"
_interpolation = r"^(linear|nn|grid-box-average)$"
_intgrid = r"^" + g + r"|none|source$"
_truncation = r"^[1-9][0-9]*|none$"


arg = argparse.ArgumentParser()

arg.add_argument(
    "--area",
    type=_Regex(_area),
    help="sub-area to be extracted (" + _area + ")",
)

arg.add_argument(
    "--grid",
    type=_Regex(_grid),
    help="Regular latitude/longitude grids (<west-east>/<south-north> increments) or Gaussian octahedral/quasi-regular/regular grids ("
    + _grid
    + ")",
)

arg.add_argument(
    "--interpolation",
    type=_Regex(_interpolation),
    help="interpolation method (" + _interpolation + ")",
)

arg.add_argument(
    "--intgrid",
    type=_Regex(_intgrid),
    help="spectral transforms intermediate Gaussian grid (" + _intgrid + ")",
)

arg.add_argument(
    "--truncation",
    type=_Regex(_truncation),
    help="spectral transforms intermediate truncation (" + _truncation + ")",
)

g = arg.add_mutually_exclusive_group()

g.add_argument(
    "--vod2uv",
    help="Input is vorticity and divergence (vo/d), convert to vector Cartesian components (gridded u/v or spectral U/V)",
    action="store_true",
)

g.add_argument(
    "--uv2uv",
    help="Input is vector Cartesian components spectral U/V or gridded u/v",
    action="store_true",
)

# arg.add_argument("-v", "--verbose", action="count", default=0, help="Verbosity level")

arg.add_argument("grib_in", type=str, help="Input GRIB file")
arg.add_argument("grib_out", type=str, help="Output GRIB file")

args = arg.parse_args()
print(args)


options = {}
for k in ["area", "grid", "interpolation", "intgrid", "truncation"]:
    if hasattr(args, k):
        v = getattr(args, k)
        if v is not None:
            options[k] = getattr(args, k)


job = mir.Job(**options)
print("Running", job)

grib_out = mir.GribFileOutput(args.grib_out)
with open(args.grib_in, "rb") as grib_in:
    job.execute(grib_in, grib_out)
