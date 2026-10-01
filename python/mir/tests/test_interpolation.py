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


# regional grids: rotated (COSMO) and projected (swisslv95, requires PROJ)
ROTATED_LL = dict(grid=[0.25, 0.25], area=[0.75, -2.5, -1, 0.25], rotation=[-43, 10])
SWISSLV95 = dict(type="swisslv95", x=[2480000, 2840000, 20000], y=[1080000, 1300000, 20000])

REGIONAL = dict(
    regular_ll=dict(grid=[1, 1]),
    reduced_gg=dict(grid="O96"),
    icon=dict(grid="ICON-CH2"),
    rotated_ll=ROTATED_LL,
    swisslv95=SWISSLV95,
)


def _has_grid(spec) -> bool:
    try:
        return len(mir.Grid(spec)) > 0
    except RuntimeError:
        return False


@pytest.mark.parametrize("interpolation", ["nn", "linear"])
@pytest.mark.parametrize(
    "input_grid, output_grid",
    [(a, b) for a, b in product(REGIONAL, REGIONAL) if {a, b} & {"rotated_ll", "swisslv95"}],
)
def test_interpolation_rotated_and_projected(input_grid, output_grid, interpolation):
    import numpy as np

    if "swisslv95" in (input_grid, output_grid) and not _has_grid(SWISSLV95):
        pytest.skip("swisslv95 requires PROJ")

    grid = mir.Grid(REGIONAL[input_grid])
    input = mir.ArrayInput(np.arange(len(grid), dtype=np.float64), grid)

    output = mir.ArrayOutput()
    mir.Job(grid=REGIONAL[output_grid], interpolation=interpolation).execute(input, output)

    values = output.values()
    assert 0 < output.size <= len(mir.Grid(REGIONAL[output_grid]))
    assert not np.isnan(values).all()

    # rotated and projected outputs are not cropped
    if output_grid in ("rotated_ll", "swisslv95"):
        assert output.size == len(mir.Grid(REGIONAL[output_grid]))


@pytest.mark.skip(reason="ecCodes changing GRIB edition=1 to 2 loses a non-default missingValue (WIP)")
@pytest.mark.parametrize(
    "input_grid, output_grid, interpolation, size, missing",
    [
        ("swisslv95", dict(grid=[0.1, 0.1]), "linear", 960, 12),
        ("swisslv95", dict(grid=[0.25, 0.25]), "grid-box-statistics", 152, 15),
        ("rotated_ll", dict(grid=[0.1, 0.1]), "linear", 738, 19),
    ],
)
def test_interpolation_rotated_and_projected_grib(input_grid, output_grid, interpolation, size, missing):
    import numpy as np
    from yaml import dump

    # output with missing values introduced by the interpolation (input has none)
    buffer = bytearray(1 << 20)
    output = mir.GribMemoryOutput(buffer)
    input = mir.GridSpecInput(dump(REGIONAL[input_grid], default_flow_style=True))
    mir.Job(grid=output_grid, interpolation=interpolation).execute(input, output)

    result = mir.ArrayOutput()
    mir.Job().execute(mir.GribMemoryInput(bytes(buffer[: len(output)])), result)

    values = result.values()
    assert values.size == size
    assert np.isnan(values).sum() == missing


if __name__ == "__main__":
    pytest.main([__file__])
