# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

"""NetCDF (CF) 2D fields: grid, param and values (requires netCDF4)"""

import warnings
from dataclasses import dataclass
from dataclasses import field
from functools import lru_cache
from pathlib import Path
from typing import Iterator
from typing import Optional

import numpy as np
import yaml

# variables not regridded (in addition to coordinates, by their CF role)
SKIP_NAME_LIST = (
    "latitude",
    "longitude",
    "lat",
    "lon",
    "nav_lat",
    "nav_lon",
    "rlat",
    "rlon",
    "x",
    "y",
    "time",
    "time_bnds",
    "lat_bnds",
    "lon_bnds",
    "latitude_bnds",
    "longitude_bnds",
    "crs",
)


# ---- datasets and variables, by their CF role ----

# attributes naming other variables, which are then not data variables
_REFERENCING_ATTRIBUTES = ("coordinates", "bounds", "climatology", "grid_mapping", "cell_measures")


def _open(path):
    try:
        import netCDF4
    except ImportError as e:
        raise ImportError("mir.netcdf requires the netCDF4 module (pip install mir-python[netcdf])") from e

    ds = netCDF4.Dataset(path)
    ds.set_auto_maskandscale(True)
    return ds


def _attribute_names(var, name: str) -> list:
    """Variable names in an attribute, e.g. coordinates 'lat lon' or grid_mapping 'crs: x y'"""
    value = getattr(var, name, "")
    if not isinstance(value, str):
        return []

    tokens = value.split()
    if not any(t.endswith(":") for t in tokens):
        return tokens

    # forms "name: coord [coord...]" (grid_mapping) and "measure: name" (cell_measures)
    if name == "grid_mapping":
        return [t[:-1] for t in tokens if t.endswith(":")]
    return [t for t in tokens if not t.endswith(":")]


def _data_variables(ds, skip_name_list) -> list:
    excluded = {name for name, var in ds.variables.items() if var.dimensions == (name,)}
    for var in ds.variables.values():
        for attr in _REFERENCING_ATTRIBUTES:
            excluded.update(_attribute_names(var, attr))
    excluded.update(skip_name_list)
    return [name for name in ds.variables if name not in excluded]


# ---- horizontal coordinates ----

_UNITS = {
    "latitude": {"degrees_north", "degree_north", "degrees_N", "degree_N", "degreesN", "degreeN"},
    "longitude": {"degrees_east", "degree_east", "degrees_E", "degree_E", "degreesE", "degreeE"},
}

_NAMES = {
    "latitude": ("latitude", "lat", "nav_lat", "lat_rho", "gphit", "yt_ocean"),
    "longitude": ("longitude", "lon", "nav_lon", "lon_rho", "glamt", "xt_ocean"),
}


@dataclass(frozen=True)
class _Horizontal:
    latitude: str
    longitude: str
    dimensions: tuple


def _find(candidates: list, kind: str):
    """By standard_name, then units, then name"""
    for match in (
        lambda v: getattr(v, "standard_name", None) == kind,
        lambda v: getattr(v, "units", None) in _UNITS[kind],
        lambda v: v.name in _NAMES[kind],
    ):
        for var in candidates:
            if match(var):
                return var
    return None


def _horizontal(ds, var) -> Optional[_Horizontal]:
    names = [d for d in var.dimensions if d in ds.variables] + _attribute_names(var, "coordinates")
    candidates = [ds.variables[n] for n in dict.fromkeys(names) if n in ds.variables]

    lat = _find(candidates, "latitude")
    lon = _find(candidates, "longitude")
    if lat is None or lon is None:
        return None

    dims = set(lat.dimensions) | set(lon.dimensions)
    if not dims or len(dims) > len(var.dimensions):
        return None

    trailing = var.dimensions[len(var.dimensions) - len(dims) :]
    if set(trailing) != dims:
        return None

    return _Horizontal(lat.name, lon.name, tuple(trailing))


# ---- grid: regular_ll, irregular_ll, or unstructured (in that order of preference) ----

# a coordinate matches a regular axis if within this fraction of the increment (coordinates are often float32)
_TOLERANCE = 1e-3


def _snap(x: float, tolerance: float) -> float:
    """Shortest decimal within tolerance (e.g. 0.2 for float32 0.2000000030)"""
    x = float(x)
    for digits in range(13):
        if abs(round(x, digits) - x) <= tolerance:
            return round(x, digits)
    return x


def _snap_coordinate(x: float, precision: float) -> float:
    return _snap(x, precision * max(1.0, abs(x)))


def _unmasked(var, index=Ellipsis):
    """Not masked, as valid_min/valid_max of coordinates are often rounded"""
    var.set_auto_mask(False)
    return var[index]


def _coordinates(var) -> np.ndarray:
    return np.asarray(_unmasked(var), dtype=np.float64)


def _precision(*variables) -> float:
    return max(4 * float(np.finfo(v.dtype).eps) if v.dtype.kind == "f" else 0.0 for v in variables)


def _increment(axis: np.ndarray, precision: float) -> Optional[float]:
    if axis.size < 2:
        return None

    first, last = _snap_coordinate(axis[0], precision), _snap_coordinate(axis[-1], precision)
    increment = _snap((last - first) / (axis.size - 1), precision)
    if increment == 0:
        return None

    expected = axis[0] + increment * np.arange(axis.size)
    return increment if np.all(np.abs(axis - expected) <= _TOLERANCE * abs(increment)) else None


def _order(lat: np.ndarray, lon: np.ndarray, lat_first: bool) -> str:
    """Directions of i (longitude) and j (latitude), the consecutive one (last dimension) first"""
    i = "i" + ("+" if lon[-1] > lon[0] else "-")
    j = "j" + ("+" if lat[-1] > lat[0] else "-")
    return i + j if lat_first else j + i


def _validate(grid: dict, lat: np.ndarray, lon: np.ndarray) -> bool:
    """The grid points are those of the axes"""
    from eckit.geo import Grid

    try:
        g = Grid(grid)
    except Exception:
        return False

    # distinct latitudes/longitudes are in the scanning directions
    glat, glon = np.sort(g.distinct_latitudes()), np.sort(g.distinct_longitudes())
    if glat.size != lat.size or glon.size != lon.size:
        return False

    tolerance = _TOLERANCE * min(grid["grid"])
    dlon = (glon - np.sort(lon) + 180.0) % 360.0 - 180.0
    return bool(np.all(np.abs(glat - np.sort(lat)) <= tolerance) and np.all(np.abs(dlon) <= tolerance))


def _regular_ll(lat: np.ndarray, lon: np.ndarray, lat_first: bool, precision: float) -> Optional[dict]:
    dlat = _increment(lat, precision)
    dlon = _increment(lon, precision)
    if dlat is None or dlon is None:
        return None

    lat0, lat1, lon0, lon1 = (_snap_coordinate(x, precision) for x in (lat[0], lat[-1], lon[0], lon[-1]))
    north, south = max(lat0, lat1), min(lat0, lat1)
    west, east = min(lon0, lon1), max(lon0, lon1)

    grid = {
        "type": "regular_ll",
        "grid": [abs(dlon), abs(dlat)],
        "area": [north, west, south, east],
        "order": _order(lat, lon, lat_first),
    }
    return grid if _validate(grid, lat, lon) else None


def _irregular_ll(lat: np.ndarray, lon: np.ndarray, lat_first: bool) -> Optional[dict]:
    def monotonic(v):
        d = np.diff(v)
        return np.all(d > 0) or np.all(d < 0)

    if lat.size < 2 or lon.size < 2 or not monotonic(lat) or not monotonic(lon):
        return None

    return {
        "type": "irregular_ll",
        "latitudes": lat.tolist(),
        "longitudes": lon.tolist(),
        "order": _order(lat, lon, lat_first),
    }


def _grid(ds, h: _Horizontal) -> dict:
    """Points in the variable (stored) order"""
    lat_var, lon_var = ds.variables[h.latitude], ds.variables[h.longitude]
    lat, lon = _coordinates(lat_var), _coordinates(lon_var)

    # rectilinear: 1-D latitude and longitude, on different dimensions
    if lat_var.ndim == 1 and lon_var.ndim == 1 and lat_var.dimensions != lon_var.dimensions:
        lat_first = h.dimensions.index(lat_var.dimensions[0]) < h.dimensions.index(lon_var.dimensions[0])
        grid = _regular_ll(lat, lon, lat_first, _precision(lat_var, lon_var)) or _irregular_ll(lat, lon, lat_first)
        if grid is None:
            raise ValueError(f"{h}: latitude/longitude are neither regular nor monotonic")
        return grid

    # curvilinear (2-D) or cells (1-D): latitude and longitude on the same dimensions, as the variable
    if lat_var.dimensions != lon_var.dimensions or set(lat_var.dimensions) != set(h.dimensions):
        raise ValueError(f"{h}: latitude/longitude dimensions {lat_var.dimensions}/{lon_var.dimensions} not supported")

    axes = [lat_var.dimensions.index(d) for d in h.dimensions]
    lat, lon = np.transpose(lat, axes).ravel(), np.transpose(lon, axes).ravel()
    return {"latitudes": lat.tolist(), "longitudes": lon.tolist()}


# ---- param: metadata of a 2D field (a variable at an index of its non-horizontal dimensions) ----

_ATTRIBUTES = ("standard_name", "long_name", "units", "cell_methods")

_VERTICAL_STANDARD_NAMES = {
    "air_pressure",
    "altitude",
    "atmosphere_hybrid_sigma_pressure_coordinate",
    "depth",
    "height",
    "model_level_number",
}


@lru_cache(maxsize=None)
def _standard_names() -> dict:
    with open(Path(__file__).parent / "netcdf.yaml") as f:
        return yaml.safe_load(f)["param"]


def _is_time(var) -> bool:
    units = getattr(var, "units", "")
    return (
        getattr(var, "axis", None) == "T"
        or getattr(var, "standard_name", None) == "time"
        or (isinstance(units, str) and " since " in units)
    )


def _is_vertical(var) -> bool:
    return (
        getattr(var, "axis", None) == "Z"
        or hasattr(var, "positive")
        or getattr(var, "standard_name", None) in _VERTICAL_STANDARD_NAMES
    )


def _scalar(value):
    value = np.ma.getdata(value)
    return value.item() if hasattr(value, "item") else value


def _coordinate(var, value) -> tuple:
    if _is_time(var):
        from cftime import num2date  # a netCDF4 dependency

        # the time axis as "time", other times (e.g. forecast_reference_time) by their standard name
        standard_name = getattr(var, "standard_name", None)
        key = "time" if getattr(var, "axis", None) == "T" or standard_name == "time" else standard_name or var.name
        return key, num2date(_scalar(value), var.units, calendar=getattr(var, "calendar", "standard")).isoformat()

    entry = {"name": var.name, "value": _scalar(value)}
    entry.update({a: getattr(var, a) for a in ("units", "positive") if hasattr(var, a)})
    return ("level", entry) if _is_vertical(var) else (var.name, entry)


def _param(ds, var, index: dict) -> dict:
    p = {"name": var.name}
    p.update({a: getattr(var, a) for a in _ATTRIBUTES if hasattr(var, a)})

    # scalar coordinates (e.g. height = 2 m), then the dimensions at the index
    for name in _attribute_names(var, "coordinates"):
        if name in ds.variables and ds.variables[name].ndim == 0:
            key, value = _coordinate(ds.variables[name], _unmasked(ds.variables[name]))
            p[key] = value

    for dim, position in index.items():
        if dim in ds.variables:
            key, value = _coordinate(ds.variables[dim], _unmasked(ds.variables[dim], position))
            p[key] = value
        else:
            p[dim] = {"index": position}

    p.update(_standard_names().get(p.get("standard_name"), {}))
    return p


# ---- fields ----


@dataclass(frozen=True)
class Field:
    """Values are read while the file is open"""

    name: str
    grid: dict
    param: dict
    _var: object = field(repr=False, compare=False)
    _index: tuple = field(repr=False, compare=False)

    def values(self, dtype=np.float64) -> np.ndarray:
        """NaN where missing"""
        if not np.issubdtype(dtype, np.floating):
            raise TypeError(f"mir.netcdf: values dtype must be floating point, not {dtype}")

        data = np.ma.asarray(self._var[self._index + (Ellipsis,)], dtype=dtype)
        return np.ascontiguousarray(np.ma.filled(data, np.nan).ravel())


def _geometries(ds, skip_name_list) -> Iterator[tuple]:
    grids = {}
    for name in _data_variables(ds, skip_name_list):
        var = ds.variables[name]
        h = _horizontal(ds, var)
        if h is None:
            warnings.warn(f"mir.netcdf: '{name}' has no latitude/longitude coordinates, skipped")
            continue
        if h not in grids:
            grids[h] = _grid(ds, h)
        yield var, h, grids[h]


def grids(path, skip_name_list=SKIP_NAME_LIST) -> dict:
    """Grid of each data variable (by name)"""
    with _open(path) as ds:
        return {var.name: grid for var, _, grid in _geometries(ds, skip_name_list)}


def fields(path, skip_name_list=SKIP_NAME_LIST) -> Iterator[Field]:
    """2D fields over the non-horizontal dimensions, the file is open while iterating"""
    with _open(path) as ds:
        for var, h, grid in _geometries(ds, skip_name_list):
            leading = var.dimensions[: var.ndim - len(h.dimensions)]
            for index in np.ndindex(var.shape[: len(leading)]):
                yield Field(var.name, grid, _param(ds, var, dict(zip(leading, index))), var, index)
