# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0


# Interpolation between global and regional grids, all combinations of input/output grids and interpolation methods:
# - nn and linear work for all combinations
# - grid-box-statistics requires output grid boxes (regular_ll, reduced_gg)
# - grid-box-average requires input and output grid boxes


from itertools import product

import numpy as np
import pytest

import mir

# regional grids: rotated (COSMO) and projected (swisslv95, requires PROJ), output grids differ from input grids
INPUT = dict(
    regular_ll=dict(grid=[2, 2]),
    reduced_gg=dict(grid="O64"),
    icon=dict(grid="ICON-CH2"),
    rotated_ll=dict(grid=[0.25, 0.25], area=[0.75, -2.5, -1, 0.25], rotation=[-43, 10]),
    swisslv95=dict(type="swisslv95", x=[2480000, 2840000, 20000], y=[1080000, 1300000, 20000]),
)

OUTPUT = dict(
    regular_ll=dict(grid=[1, 1]),
    reduced_gg=dict(grid="O96"),
    icon=dict(grid="ICON-CH2"),
    rotated_ll=dict(grid=[0.5, 0.5], area=[0.5, -2, -0.5, 0], rotation=[-43, 10]),
    swisslv95=dict(type="swisslv95", x=[2480000, 2840000, 40000], y=[1080000, 1280000, 40000]),
)

INTERPOLATIONS = ["nn", "linear", "grid-box-statistics", "grid-box-average"]

GRID_BOXES = ("regular_ll", "reduced_gg")

# should work, but doesn't
SHOULD_WORK_REASON = "regional rotated input domain is compared (to output's) in rotated coordinates"
SHOULD_WORK = {
    ("rotated_ll", "reduced_gg", "grid-box-statistics"): SHOULD_WORK_REASON,
    ("rotated_ll", "regular_ll", "grid-box-statistics"): SHOULD_WORK_REASON,
}


def _works(input_grid, output_grid, interpolation) -> bool:
    if interpolation == "grid-box-statistics":
        return output_grid in GRID_BOXES
    if interpolation == "grid-box-average":
        return input_grid in GRID_BOXES and output_grid in GRID_BOXES
    return True


def _has_grid(spec) -> bool:
    try:
        return len(mir.Grid(spec)) > 0
    except RuntimeError:
        return False


def _cases():
    for input_grid, output_grid, interpolation in product(INPUT, OUTPUT, INTERPOLATIONS):
        if reason := SHOULD_WORK.get((input_grid, output_grid, interpolation)):
            marks = pytest.mark.xfail(strict=True, raises=RuntimeError, reason=reason)
            yield pytest.param(input_grid, output_grid, interpolation, True, marks=marks)
        else:
            yield input_grid, output_grid, interpolation, _works(input_grid, output_grid, interpolation)


def _interpolate(input_spec, output_spec, interpolation):
    grid = mir.Grid(input_spec)
    input = mir.ArrayInput(np.arange(len(grid), dtype=np.float64), grid)

    output = mir.ArrayOutput()
    mir.Job(grid=output_spec, interpolation=interpolation).execute(input, output)
    return output


@pytest.mark.parametrize("input_grid, output_grid, interpolation, works", list(_cases()))
def test_interpolation_matrix(input_grid, output_grid, interpolation, works):
    if "swisslv95" in (input_grid, output_grid) and not _has_grid(INPUT["swisslv95"]):
        pytest.skip("swisslv95 requires PROJ")

    if not works and mir.Grid(INPUT[input_grid]) == mir.Grid(OUTPUT[output_grid]):
        pytest.skip("same input/output grid, not interpolated")

    if not works:
        with pytest.raises(RuntimeError):
            _interpolate(INPUT[input_grid], OUTPUT[output_grid], interpolation)
        return

    output = _interpolate(INPUT[input_grid], OUTPUT[output_grid], interpolation)

    assert 0 < output.size <= len(mir.Grid(OUTPUT[output_grid]))
    assert not np.isnan(output.values()).all()

    # rotated and projected outputs are not cropped
    if output_grid in ("rotated_ll", "swisslv95"):
        assert output.size == len(mir.Grid(OUTPUT[output_grid]))


# grid-box-statistics from rotated grids: the input domain is checked to contain the output's
@pytest.mark.parametrize(
    "input_spec, output_spec, options",
    [
        # global (always contains the output)
        pytest.param(dict(grid=[2, 2], rotation=[-40, 20]), dict(grid=[5, 5]), {}, id="global-to-regular_ll"),
        pytest.param(dict(grid=[2, 2], rotation=[-40, 20]), dict(grid="O32"), {}, id="global-to-reduced_gg"),
        # regional, containing the output
        pytest.param(
            INPUT["rotated_ll"],
            dict(grid=[0.1, 0.1], area=[47, 7, 46.5, 8]),
            {},
            marks=pytest.mark.xfail(strict=True, raises=RuntimeError, reason=SHOULD_WORK_REASON),
            id="regional-to-regular_ll",
        ),
        pytest.param(
            INPUT["rotated_ll"],
            dict(grid=[0.1, 0.1], area=[47, 7, 46.5, 8]),
            dict(interpolation_global_input="true"),
            marks=pytest.mark.xfail(strict=True, raises=RuntimeError, reason=SHOULD_WORK_REASON),
            id="regional-to-regular_ll-interpolation-global-input",
        ),
    ],
)
def test_interpolation_rotated_grid_box_statistics(input_spec, output_spec, options):
    grid = mir.Grid(input_spec)
    input = mir.ArrayInput(np.arange(len(grid), dtype=np.float64), grid)

    output = mir.ArrayOutput()
    mir.Job(grid=output_spec, interpolation="grid-box-statistics", **options).execute(input, output)

    assert output.size == len(mir.Grid(output_spec))
    assert not np.isnan(output.values()).all()


@pytest.mark.skip(reason="ecCodes changing GRIB edition=1 to 2 loses a non-default missingValue (WIP)")
@pytest.mark.parametrize(
    "input_grid, output_spec, interpolation, size, missing",
    [
        ("swisslv95", dict(grid=[0.1, 0.1]), "linear", 960, 12),
        ("swisslv95", dict(grid=[0.25, 0.25]), "grid-box-statistics", 152, 15),
        ("rotated_ll", dict(grid=[0.1, 0.1]), "linear", 738, 19),
    ],
)
def test_interpolation_matrix_grib(input_grid, output_spec, interpolation, size, missing):
    from yaml import dump

    # output with missing values introduced by the interpolation (input has none)
    buffer = bytearray(1 << 20)
    output = mir.GribMemoryOutput(buffer)
    input = mir.GridSpecInput(dump(INPUT[input_grid], default_flow_style=True))
    mir.Job(grid=output_spec, interpolation=interpolation).execute(input, output)

    result = mir.ArrayOutput()
    mir.Job().execute(mir.GribMemoryInput(bytes(buffer[: len(output)])), result)

    values = result.values()
    assert values.size == size
    assert np.isnan(values).sum() == missing


if __name__ == "__main__":
    pytest.main([__file__])
