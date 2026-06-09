#!/usr/bin/env python3

# (C) Copyright 1996- ECMWF.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
#
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation nor
# does it submit to any jurisdiction.


import numpy as np
from mir.weight_matrix import WeightMatrix
from scipy.sparse import csr_array

import mir

# --- interpolation ---

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


#  --- weight matrix handling ---

# interpolation-like matrix
#  [  .   0.6   .   0.4   .  ]
#  [  .    .   1.0   .    .  ]
#  [ 0.3   .    .    .   0.7 ]
#  [  .   0.2  0.5  0.3   .  ]
data    = np.array([0.6, 0.4, 1.0, 0.3, 0.7, 0.2, 0.5, 0.3])
indices = np.array([1,   3,   2,   0,   4,   1,   2,   3  ])
indptr  = np.array([0,   2,   3,   5,   8])
wm = WeightMatrix.from_csr(csr_array((data, indices, indptr), shape=(4, 5)))

# modify row 2, apply to a field
lil = wm.to_csr().tolil()
lil[2, 0] = 1.0
lil[2, 4] = 0.0
modified = csr_array(lil)
modified.eliminate_zeros()
wm2 = WeightMatrix.from_csr(modified)

field = np.array([1.0, 2.0, 3.0, 4.0, 5.0])

result = wm2.to_csr() @ field

print(result)
assert np.allclose(result, [2.0, 3.0, 1.0, 3.4])


# modify row 2 again, apply to a field
row_i = 2
start, end = modified.indptr[row_i], modified.indptr[row_i + 1]
for col, val in zip(modified.indices[start:end], modified.data[start:end]):
    pass  # col=0, val=1.0

lil3 = modified.tolil()
lil3[2, 0] = 0.1
lil3[2, 4] = 0.9
modified2 = csr_array(lil3)
wm3 = WeightMatrix.from_csr(modified2)

result2 = wm3.to_csr() @ field

# row 2: 0.1*1 + 0.9*5 = 4.6
assert np.allclose(result2[2], 4.6)
