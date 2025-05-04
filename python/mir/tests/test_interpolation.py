# (C) Copyright 1996- ECMWF.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
#
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation nor
# does it submit to any jurisdiction.


from itertools import product

import mir
import pytest

GRIDSPECS = [
    (dict(grid="1/1"), dict(grid=[1, 1]), (181, 360)),
    (dict(grid="h2n"), dict(grid="H2", ordering="nested"), (48,)),
    (dict(grid="H3", ordering="ring"), dict(grid="H3"), (108,)),
    (dict(grid=[2, 2]), dict(grid=[2, 2]), (91, 180)),
    ("{grid: 3/3}", dict(grid=[3, 3]), (61, 120)),
    (dict(grid="o2"), dict(grid="O2"), (sum([20, 24, 24, 20]),)),
]


@pytest.mark.parametrize("grid, spec, shape", GRIDSPECS)
def test_shapes_and_specs(grid, spec, shape):
    grid = mir.Grid(grid)
    assert grid.shape == shape
    assert grid.spec == spec


@pytest.mark.parametrize(
    "input_grid, output_grid, output_spec, output_shape",
    [
        (a[0], b[0], b[1], b[2])
        for a, b in product(GRIDSPECS, GRIDSPECS)
        if a != b
    ],
)
def test_interpolation_on_array(
    input_grid, output_grid, output_spec, output_shape
):
    from array import array

    grid = mir.Grid(input_grid)
    data = array("d", list(range(len(grid))))
    input = mir.ArrayInput(data, grid.spec_str)

    job = mir.Job()
    job.set("grid", output_grid)

    output = mir.ArrayOutput()
    job.execute(input, output)

    result = mir.Grid(output.spec)
    assert result.shape == output_shape
    assert result.spec == output_spec

    assert output.shape == output_shape
    assert output.values(typecode="d").typecode == "d"
    assert output.values(typecode="f").typecode == "f"
    assert len(output.values(typecode="d")) == len(result)
    assert len(output.values(typecode="d")) == output.size == len(result)

    data = array("f", list(range(len(grid))))
    input = mir.ArrayInput(data, grid.spec_str)

    job.execute(input, output)

    result = mir.Grid(output.spec)
    assert result.shape == output_shape
    assert output.shape == output_shape
    assert len(output.values(typecode="d")) == output.size == len(result)


@pytest.mark.parametrize(
    "input_grid, output_grid, output_spec, output_shape",
    [
        (a[0], b[0], b[1], b[2])
        for a, b in product(GRIDSPECS, GRIDSPECS)
        if a != b
    ],
)
def test_interpolation_on_nparray(
    input_grid, output_grid, output_spec, output_shape
):
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


if __name__ == "__main__":
    pytest.main([__file__])
