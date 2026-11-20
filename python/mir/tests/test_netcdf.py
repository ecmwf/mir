# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0


import sys
from pathlib import Path

import mir.netcdf
import numpy as np
import pytest

import mir

FILL = -32767


def write_netcdf(path: Path, lat, lon, variables: dict, times=None, lat_dtype="f4") -> Path:
    """Small CF netCDF file: variables {name: (dtype, data, attributes)} on (time,) latitude, longitude"""
    netCDF4 = pytest.importorskip("netCDF4")

    with netCDF4.Dataset(path, "w") as ds:
        dims = ("latitude", "longitude")
        if times is not None:
            ds.createDimension("time", None)
            t = ds.createVariable("time", "i4", ("time",))
            t.setncatts({"standard_name": "time", "units": "hours since 1950-01-01 00:00:00.0", "calendar": "gregorian"})
            t[:] = times
            dims = ("time",) + dims

        for name, values, units in (("latitude", lat, "degrees_north"), ("longitude", lon, "degrees_east")):
            ds.createDimension(name, len(values))
            v = ds.createVariable(name, lat_dtype, (name,))
            v.setncatts({"standard_name": name, "units": units})
            v[:] = values

        for name, (dtype, data, attributes) in variables.items():
            fill = attributes.pop("_FillValue", None)
            v = ds.createVariable(name, dtype, dims[-np.ndim(data) :], fill_value=fill)
            v.setncatts(attributes)
            v.set_auto_maskandscale(False)
            v[:] = data
    return path


# like WAVERYS: float32 0.2 degree axes, latitudes increasing (south to north)
LAT_02 = np.linspace(10.0, 11.0, 6, dtype=np.float32)
LON_02 = np.linspace(-1.0, 0.4, 8, dtype=np.float32)

# like WAVERYS, global layout: longitudes from -180, no pole rows
LAT_20 = np.linspace(-80.0, 80.0, 9, dtype=np.float32)
LON_20 = np.linspace(-180.0, 160.0, 18, dtype=np.float32)


def read(path, **kwargs) -> list:
    """Values are read while the file is open"""
    return [(f, f.values()) for f in mir.netcdf.fields(path, **kwargs)]


def test_regular_ll_float32_increments(tmp_path):
    # exact increments would fail (float32), the grid is regular_ll nevertheless
    assert len(set(np.diff(LAT_02.astype(np.float64)))) > 1

    data = np.zeros((LAT_02.size, LON_02.size))
    path = write_netcdf(tmp_path / "a.nc", LAT_02, LON_02, {"VHM0": ("f8", data, {})})
    ((field, values),) = read(path)

    assert field.grid == {"type": "regular_ll", "grid": [0.2, 0.2], "area": [11.0, -1.0, 10.0, 0.4], "order": "i+j+"}
    assert values.shape == (LAT_02.size * LON_02.size,)
    assert values.dtype == np.float64


@pytest.mark.parametrize("lat, order", [(LAT_20, "i+j+"), (LAT_20[::-1], "i+j-")])
def test_regular_ll_global_order(tmp_path, lat, order):
    data = np.zeros((lat.size, LON_20.size))
    path = write_netcdf(tmp_path / "a.nc", lat, LON_20, {"deptho": ("f8", data, {})})
    ((field, _),) = read(path)

    assert field.grid == {"type": "regular_ll", "grid": [20.0, 20.0], "area": [80.0, -180.0, -80.0, 160.0], "order": order}

    # the grid points follow the stored order
    grid = mir.Grid(field.grid)
    assert grid.shape == (9, 18)

    glat, glon = grid.to_latlons()
    assert np.allclose(glat[:: LON_20.size], lat)
    assert np.allclose(glon[: LON_20.size], LON_20)


def test_missing_values(tmp_path):
    # like the bathymetry: double, with both _FillValue and missing_value
    lat = LAT_20[::-1]
    data = np.arange(lat.size * LON_20.size, dtype=np.float64).reshape(lat.size, LON_20.size)
    missing = np.zeros(data.shape, dtype=bool)
    missing[0, :3] = missing[4, 7] = True
    data[missing] = FILL

    attributes = {"_FillValue": float(FILL), "missing_value": float(FILL)}
    path = write_netcdf(tmp_path / "a.nc", lat, LON_20, {"deptho": ("f8", data, attributes)})
    ((_, values),) = read(path)

    assert np.array_equal(np.isnan(values), missing.ravel())
    assert np.array_equal(values[~missing.ravel()], data[~missing])


def test_missing_values_packed(tmp_path):
    # like VMDR: short, scale_factor/add_offset and _FillValue (packed)
    raw = np.arange(LAT_20.size * LON_20.size, dtype=np.int16).reshape(LAT_20.size, LON_20.size)
    raw[2, 5] = FILL

    attributes = {"_FillValue": np.int16(FILL), "scale_factor": 0.01, "add_offset": 180.0}
    path = write_netcdf(tmp_path / "a.nc", LAT_20, LON_20, {"VMDR": ("i2", raw, attributes)})
    (values,) = [f.values(np.float32) for f in mir.netcdf.fields(path)]

    expected = raw.ravel() * 0.01 + 180.0
    missing = raw.ravel() == FILL
    assert values.dtype == np.float32
    assert np.array_equal(np.isnan(values), missing)
    assert np.allclose(values[~missing], expected[~missing])


def test_values_dtype(tmp_path):
    data = np.zeros((LAT_20.size, LON_20.size))
    path = write_netcdf(tmp_path / "a.nc", LAT_20, LON_20, {"a": ("f8", data, {})})
    with pytest.raises(TypeError):
        for f in mir.netcdf.fields(path):
            f.values(np.int32)


def test_time_and_param(tmp_path):
    hours = 631152  # 2022-01-01T00:00 in hours since 1950-01-01
    data = np.zeros((3, LAT_20.size, LON_20.size))
    attributes = {"standard_name": "sea_surface_wave_significant_height", "units": "m"}

    times = [hours, hours + 3, hours + 6]
    path = write_netcdf(tmp_path / "a.nc", LAT_20, LON_20, {"VHM0": ("f4", data, attributes)}, times=times)
    fields = [f for f, _ in read(path)]

    assert [f.param["time"] for f in fields] == ["2022-01-01T00:00:00", "2022-01-01T03:00:00", "2022-01-01T06:00:00"]
    for f in fields:
        assert f.param["name"] == "VHM0"
        assert f.param["standard_name"] == attributes["standard_name"]
        assert f.param["units"] == "m"
        assert f.param["paramId"] == 140229


def test_skip_name_list(tmp_path):
    netCDF4 = pytest.importorskip("netCDF4")

    data = np.zeros((1, LAT_20.size, LON_20.size))
    path = write_netcdf(tmp_path / "a.nc", LAT_20, LON_20, {"a": ("f8", data, {}), "b": ("f8", data, {})}, times=[0])
    with netCDF4.Dataset(path, "a") as ds:
        ds.createVariable("time_only", "f8", ("time",))[:] = [1.0]

    # coordinates (latitude, longitude, time) are not fields, nor variables without latitude/longitude
    with pytest.warns(UserWarning, match="time_only"):
        assert [f.name for f, _ in read(path)] == ["a", "b"]

    with pytest.warns(UserWarning, match="time_only"):
        assert [f.name for f, _ in read(path, skip_name_list=["b"])] == ["a"]

    assert read(path, skip_name_list=["a", "b", "time_only"]) == []


def test_array_input_from_netcdf_file(tmp_path):
    data = np.zeros((2, LAT_20.size, LON_20.size))
    path = write_netcdf(tmp_path / "a.nc", LAT_20, LON_20, {"VHM0": ("f8", data, {"units": "m"})}, times=[0, 3])

    inputs = list(mir.ArrayInput.from_netcdf_file(path))
    assert len(inputs) == 2
    assert [i.param["time"] for i in inputs] == ["1950-01-01T00:00:00", "1950-01-01T03:00:00"]
    assert all(mir.Grid(i.grid).shape == (9, 18) for i in inputs)


def write_curvilinear(path: Path, lat, lon) -> Path:
    """Small curvilinear netCDF file (2-D latitude/longitude, like NEMO), a field of the point indices"""
    netCDF4 = pytest.importorskip("netCDF4")

    with netCDF4.Dataset(path, "w") as ds:
        ds.createDimension("y", lat.shape[0])
        ds.createDimension("x", lat.shape[1])
        for name, values, units in (("nav_lat", lat, "degrees_north"), ("nav_lon", lon, "degrees_east")):
            v = ds.createVariable(name, "f4", ("y", "x"))
            v.units = units
            v[:] = values
        v = ds.createVariable("field", "f8", ("y", "x"))
        v.coordinates = "nav_lat nav_lon"
        v[:] = np.arange(lat.size, dtype=np.float64).reshape(lat.shape)
    return path


def test_unstructured(tmp_path):
    lon, lat = np.meshgrid(np.array([0.0, 120.0, 240.0, 300.0]), np.array([-30.0, 0.0, 30.0]))
    path = write_curvilinear(tmp_path / "a.nc", lat, lon)
    ((field, values),) = read(path)

    assert field.grid == {"latitudes": lat.ravel().tolist(), "longitudes": lon.ravel().tolist()}

    out = mir.ArrayOutput()
    mir.Job().execute(mir.ArrayInput(values, field.grid), out)
    assert np.array_equal(out.values().ravel(), values)


def test_netcdf4_not_installed(monkeypatch, tmp_path):
    monkeypatch.setitem(sys.modules, "netCDF4", None)
    with pytest.raises(ImportError, match="netCDF4"):
        list(mir.netcdf.fields(tmp_path / "a.nc"))


if __name__ == "__main__":
    pytest.main([__file__])
