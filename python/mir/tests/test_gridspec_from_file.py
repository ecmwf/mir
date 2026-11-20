# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0


from pathlib import Path

import numpy as np
import pytest
from test_netcdf import LAT_20
from test_netcdf import LON_20
from test_netcdf import write_netcdf

import mir

TEST_DIR = Path(__file__).parent


def test_netcdf(tmp_path):
    data = np.zeros((LAT_20.size, LON_20.size))
    path = write_netcdf(tmp_path / "a.nc", LAT_20[::-1], LON_20, {"a": ("f8", data, {}), "b": ("f4", data, {})})

    grid = {"type": "regular_ll", "grid": [20.0, 20.0], "area": [80.0, -180.0, -80.0, 160.0], "order": "i+j-"}
    assert mir.gridspec_from_file(path) == {"a": grid, "b": grid}


def write_grib(path: Path, scanning_mode: int) -> Path:
    """Small regular_ll GRIB (20 degree, latitudes 80 to -80, or -80 to 80 if j scans positively)"""
    eccodes = pytest.importorskip("eccodes")

    north_first = scanning_mode & 64 == 0
    h = eccodes.codes_grib_new_from_samples("regular_ll_sfc_grib2")
    for key, value in (
        ("Ni", 18),
        ("Nj", 9),
        ("iDirectionIncrementInDegrees", 20.0),
        ("jDirectionIncrementInDegrees", 20.0),
        ("latitudeOfFirstGridPointInDegrees", 80.0 if north_first else -80.0),
        ("longitudeOfFirstGridPointInDegrees", 0.0),
        ("latitudeOfLastGridPointInDegrees", -80.0 if north_first else 80.0),
        ("longitudeOfLastGridPointInDegrees", 340.0),
        ("scanningMode", scanning_mode),
    ):
        eccodes.codes_set(h, key, value)
    eccodes.codes_set_values(h, np.zeros(18 * 9))

    with open(path, "wb") as f:
        eccodes.codes_write(h, f)
    eccodes.codes_release(h)
    return path


def grib_grids(path) -> dict:
    try:
        return mir.gridspec_from_file(path)
    except RuntimeError as e:
        pytest.skip(str(e))  # eccodes without eckit::geo


@pytest.mark.parametrize("scanning_mode, grid", [(0, {"grid": [20, 20]}), (64, {"grid": [20, 20], "order": "i+j+"})])
def test_grib_regular_ll(tmp_path, scanning_mode, grid):
    path = write_grib(tmp_path / "a.grib2", scanning_mode)
    assert grib_grids(path) == {"1:t": grid}


def test_grib_reduced_gg():
    pytest.importorskip("eccodes")
    assert list(grib_grids(TEST_DIR / "o96.grib2").values()) == [{"grid": "O96"}]


def test_unknown_format(tmp_path):
    path = tmp_path / "a.txt"
    path.write_text("neither GRIB nor netCDF")
    with pytest.raises(ValueError):
        mir.gridspec_from_file(path)


if __name__ == "__main__":
    pytest.main([__file__])
