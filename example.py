# (C) Copyright 1996- ECMWF.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
#
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation nor
# does it submit to any jurisdiction.

import mir

# Retrieve some data as GRIB, e.g.
#
#   echo retrieve,param=z,target=z.grib | mars

# Create a MIR Job
job = mir.Job(grid="1.0/1.0")

# Run the job on the retrieved GRIB file
print("Running", job)
print("File to file")
job.execute(mir.GribFileInput("z.grib"), mir.GribFileOutput("z_ll_ff.grib"))

print("Memory to file")
with open("z.grib", "rb") as f:
    data = f.read()
job.execute(mir.GribMemoryInput(data), mir.GribFileOutput("z_ll_mf.grib"))

print("File to memory")
buf = bytearray(64 * 1024 * 1024)
mem_out = mir.GribMemoryOutput(buf)
job.execute(mir.GribFileInput("z.grib"), mem_out)
with open("z_ll_fm.grib", "wb") as f:
    f.write(buf[: len(mem_out)])

print("File-like object to file")
with open("z.grib", "rb") as f:
    job.execute(f, mir.GribFileOutput("z_ll_of.grib"))

print("File to file-like object")
with open("z_ll_fo.grib", "wb") as f:
    job.execute(mir.GribFileInput("z.grib"), f)
