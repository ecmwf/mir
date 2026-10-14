# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0


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


def test_array_input_gridspec_forms():
    import numpy as np

    # the same input grid, given as a string, a dict, a Grid, or its points (unstructured)
    grid = mir.Grid(dict(grid="O32"))
    lats, lons = grid.to_latlons()
    values = np.random.default_rng(0).random(grid.shape)

    job = mir.Job(grid="1/1", interpolation="nn")

    def interpolate(gridspec):
        output = mir.ArrayOutput()
        job.execute(mir.ArrayInput(values, gridspec), output)
        return output.values()

    expected = interpolate(grid.spec_str)
    for gridspec in (grid.spec, grid, mir.Grid(dict(latitudes=lats, longitudes=lons))):
        assert np.array_equal(interpolate(gridspec), expected)


def test_input_reused():
    import numpy as np

    grid = mir.Grid(dict(grid="O32"))
    values = np.arange(len(grid), dtype=np.float64)

    for input in (mir.ArrayInput(values, grid), mir.GridSpecInput(grid.spec_str)):
        for output_grid in (dict(grid=[1, 1]), dict(grid=[2, 2])):
            output = mir.ArrayOutput()
            mir.Job(grid=output_grid).execute(input, output)
            assert output.size == len(mir.Grid(output_grid))


def test_array_output_empty():
    assert mir.ArrayOutput().values().size == 0


# HEALPix ring <-> nested is a reordering of the same points, not an interpolation
@pytest.mark.parametrize("order_a, order_b", list(product(("ring", "nested"), repeat=2)))
def test_healpix_reorder(order_a, order_b):
    import numpy as np

    a = mir.Grid(dict(grid="H4", order=order_a))
    b = mir.Grid(dict(grid="H4", order=order_b))
    values = np.arange(len(a), dtype=np.float64)

    output = mir.ArrayOutput()
    mir.Job(grid=b.spec).execute(mir.ArrayInput(values, a), output)
    assert output.spec == b.spec

    result = output.values()
    assert np.array_equal(np.sort(result), values)
    assert np.array_equal(result, values) == (order_a == order_b)

    # each output point carries the value of the same input point
    lat_a, lon_a = (np.ravel(x) for x in a.to_latlons())
    lat_b, lon_b = (np.ravel(x) for x in b.to_latlons())
    index = result.astype(int)
    assert lat_a[index] == pytest.approx(lat_b)
    assert lon_a[index] == pytest.approx(lon_b)

    back = mir.ArrayOutput()
    mir.Job(grid=a.spec).execute(mir.ArrayInput(result, b), back)
    assert np.array_equal(back.values(), values)


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


@pytest.mark.parametrize(
    "gridspec, output_spec, output_shape",
    [
        (
            # rotated, the area not on the grid
            '{"area":[3.36,-6.82,-4.42,4.8],"grid":[0.5,0.5],"order":"i+j+",'
            '"projection":{"south_pole":[10,-43],"type":"rotation"}}',
            dict(
                area=[3.08, -6.82, -4.42, 4.68],
                grid=[0.5, 0.5],
                order="i+j+",
                projection=dict(south_pole=[10, -43], type="rotation"),
                reference=[0.18, 0.08],
            ),
            (16, 24),
        ),
        (
            # Lambert conformal conic (AROME France)
            '{"false":[921962.126,-885517.26],"projection":{"figure":"grib1","lat_0":45.8,"lat_1":45.8,"lon_0":2,'
            '"reverse":true,"type":"lcc"},"x":[0,1845000,2500],"y":[0,-1770000,-2500]}',
            dict(area=[53.5, -11.5, 37.5, 15.5], grid=[0.5, 0.5]),
            (33, 55),
        ),
    ],
)
def test_interpolation_gridspec(gridspec, output_spec, output_shape):
    import numpy as np

    values = np.arange(len(mir.Grid(gridspec)), dtype=np.float64)
    output = mir.ArrayOutput()
    mir.Job(grid=dict(grid=[0.5, 0.5]), interpolation="nn").execute(mir.ArrayInput(values, gridspec), output)

    assert output.spec == output_spec
    assert output.shape == output_shape
    assert np.isin(output.values(), values).all()


GRIDS = [
    "1/1",
    dict(grid=[2, 2]),
    "H4n",
    "ORCA2_T",
    dict(latitudes=[1, 2, 3], longitudes=[4, 5, 6]),
]


if __name__ == "__main__":
    pytest.main([__file__])
