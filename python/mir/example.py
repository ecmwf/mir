#!/usr/bin/env python3

# (C) Copyright 1996- ECMWF.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
#
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation nor
# does it submit to any jurisdiction.


import mir

job = mir.Job(grid="1.0/1.0")

# file to file (input is something like: echo retrieve,param=z,target=z.grib | mars)
job.execute(mir.GribFileInput("z.grib"), mir.GribFileOutput("z_ll_ff.grib"))

# memory to file
with open("z.grib", "rb") as f:
    data = f.read()
job.execute(mir.GribMemoryInput(data), mir.GribFileOutput("z_ll_mf.grib"))

# file to memory buffer
buf = bytearray(64 * 1024 * 1024)
mem_out = mir.GribMemoryOutput(buf)
job.execute(mir.GribFileInput("z.grib"), mem_out)
with open("z_ll_fm.grib", "wb") as f:
    f.write(buf[: len(mem_out)])

# file-like object as input
with open("z.grib", "rb") as f:
    job.execute(f, mir.GribFileOutput("z_ll_of.grib"))

# file-like object as output
with open("z_ll_fo.grib", "wb") as f:
    job.execute(mir.GribFileInput("z.grib"), f)
