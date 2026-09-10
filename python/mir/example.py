#!/usr/bin/env python3

# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0


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
