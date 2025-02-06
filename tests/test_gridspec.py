# (C) Copyright 2025- ECMWF.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation
# nor does it submit to any jurisdiction.

from pathlib import Path

import pytest
from itertools import product


GRIDSPECS = [
    (dict(grid="1/1"), 360 * 181),
    (dict(grid="H2", ordering="nested"), 12 * 2 * 2),
    (dict(grid=[2, 2]), 180 * 91),
    ("{grid: 3/3}", 120 * 61),
    (dict(grid="o2"), sum([20, 24, 24, 20])),
]


def _gridspec_as_string(g):
    from yaml import dump

    return g if isinstance(g, str) else dump(g, default_flow_style=True).strip()


@pytest.mark.parametrize(
    "GSa, Na, GSb, Nb",
    [
        (_gridspec_as_string(a[0]), int(a[1]), _gridspec_as_string(b[0]), int(b[1]))
        for a, b in product(GRIDSPECS, GRIDSPECS)
        if a != b
    ],
)
def test_mir(tmp_path, GSa, Na, GSb, Nb):
    import mir

    mat = Path(tmp_path) / "matrix.mat"
    assert not mat.exists()

    job = mir.Job()
    job.set("grid", GSb)
    job.set("interpolation-matrix", str(mat))

    job.execute(mir.GridSpecInput(GSa), mir.EmptyOutput())

    # FIXME also check matrix size
    assert mat.exists()
    assert Na == len(mir.Grid(GSa))
    assert Nb == len(mir.Grid(GSb))

    mat.unlink()
