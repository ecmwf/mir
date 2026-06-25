# (C) Copyright 1996- ECMWF.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
#
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation nor
# does it submit to any jurisdiction.


from itertools import product
from pathlib import Path

import pytest

import mir

TEST_DIR = Path(__file__).parent

GRIDSPECS = [
    (dict(grid="1/1"), dict(grid=[1, 1]), (181, 360)),
    (dict(grid="h2n"), dict(grid="H2", order="nested"), (48,)),
    (dict(grid="H4", order="nested"), dict(grid="H4", order="nested"), (192,)),
    (dict(grid="H3", order="ring"), dict(grid="H3"), (108,)),
    (dict(grid=[2, 2]), dict(grid=[2, 2]), (91, 180)),
    ("{grid: 3/3}", dict(grid=[3, 3]), (61, 120)),
    # (dict(grid="o2"), dict(grid="O2"), (sum([20, 24, 24, 20]),)),
]


@pytest.mark.parametrize("grid, spec, shape", GRIDSPECS)
def test_shapes_and_specs(grid, spec, shape):
    grid = mir.Grid(grid)
    assert grid.shape == shape
    assert grid.spec == spec


@pytest.mark.parametrize(
    "input_grid, output_grid, output_spec, output_shape",
    [(a[0], b[0], b[1], b[2]) for a, b in product(GRIDSPECS, GRIDSPECS) if a != b],
)
def test_interpolation(input_grid, output_grid, output_spec, output_shape):
    import numpy as np

    grid = mir.Grid(input_grid)
    arr = np.arange(len(grid), dtype=np.float64)
    input = mir.ArrayInput(arr, grid.spec_str)

    job = mir.Job()
    job.set("grid", output_grid)

    output = mir.ArrayOutput()
    job.execute(input, output)

    result = mir.Grid(output.spec)
    assert result.shape == output_shape
    assert result.spec == output_spec

    assert output.shape == output_shape
    assert output.values().dtype == np.float64
    assert output.values(dtype=np.float32).dtype == np.float32
    assert output.values().size == output.size == len(result)

    arr = np.arange(len(grid), dtype=np.float32)
    input = mir.ArrayInput(arr, grid.spec_str)

    job.execute(input, output)

    result = mir.Grid(output.spec)
    assert result.shape == output_shape
    assert output.shape == output_shape
    assert output.values().dtype == np.float64
    assert output.values().size == output.size == len(result)


@pytest.mark.parametrize(
    "input_gs, output_gs",
    [
        (dict(grid="O96"), dict(type="arakawa_c_um", n=96)),
        (dict(type="arakawa_c_um", n=96), dict(grid="O96")),
    ],
)
def test_interpolation_n96_o96_array(input_gs, output_gs):
    import numpy as np

    input_grid = mir.Grid(input_gs)
    expected_grid = mir.Grid(output_gs)

    values = np.arange(len(input_grid), dtype=np.float64)
    input = mir.ArrayInput(values, input_grid.spec_str)

    job = mir.Job()
    job.set("grid", output_gs)

    output = mir.ArrayOutput()
    job.execute(input, output)

    result = mir.Grid(output.spec)
    assert result.shape == expected_grid.shape
    assert result.spec == expected_grid.spec


@pytest.mark.parametrize(
    "input_filename, input_gs, output_gs",
    [
        ("o96.grib2", dict(grid="O96"), dict(type="arakawa_c_um", n=96)),
        ("n96.grib2", dict(type="arakawa_c_um", n=96), dict(grid="O96")),
    ],
)
def test_interpolation_n96_o96_grib(input_filename, input_gs, output_gs, monkeypatch):
    monkeypatch.setenv("ECCODES_ECKIT_GEO", "1")

    path = TEST_DIR / input_filename
    assert path.is_file() and path.exists()

    source = mir.GribFileInput(str(path))
    input_output = mir.ArrayOutput()
    mir.Job().execute(source, input_output)

    assert input_output.spec == mir.Grid(input_gs).spec

    input = mir.GribFileInput(str(path))
    job = mir.Job()
    job.set("grid", output_gs)

    output = mir.ArrayOutput()
    job.execute(input, output)

    assert output.spec == mir.Grid(output_gs).spec


GRIDS = [
    "1/1",
    dict(grid=[2, 2]),
    "H4n",
    "ORCA2_T",
    dict(latitudes=[1, 2, 3], longitudes=[4, 5, 6]),
]


if __name__ == "__main__":
    pytest.main([__file__])
