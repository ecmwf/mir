# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0


from itertools import product
from pathlib import Path

import pytest

GRIDSPECS = [
    (dict(grid="1/1"), 360 * 181),
    (dict(grid="H2", order="nested"), 12 * 2 * 2),
    (dict(grid=[2, 2]), 180 * 91),
    ("{grid: 3/3}", 120 * 61),
    (dict(grid="o2"), sum([20, 24, 24, 20])),
]


# (gridspec, canonical spec, shape)
CANONICAL = [
    (dict(grid="o96"), dict(grid="O96"), (40320,)),
    (dict(type="arakawa_c_um", n=96), dict(grid=[1.875, 1.25], order="i+j+", reference=[0.9375, 0.625]), (144, 192)),
    (dict(grid="H2"), dict(grid="H2"), (48,)),
    (dict(grid="H2", order="ring"), dict(grid="H2"), (48,)),
    (dict(grid="H2r"), dict(grid="H2"), (48,)),
    (dict(grid="hR2"), dict(grid="H2"), (48,)),
    (dict(grid="H2", order="nested"), dict(grid="H2", order="nested"), (48,)),
    (dict(grid="Hn2"), dict(grid="H2", order="nested"), (48,)),
    (dict(grid="h2N"), dict(grid="H2", order="nested"), (48,)),
]


@pytest.mark.parametrize("gridspec, spec, shape", CANONICAL)
def test_canonical_spec(gridspec, spec, shape):
    import mir

    grid = mir.Grid(gridspec)
    assert grid.spec == spec
    assert grid.shape == shape
    assert mir.Grid(grid.spec) == grid


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
