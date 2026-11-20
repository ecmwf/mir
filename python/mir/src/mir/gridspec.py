# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

"""Grids of the fields in a file (GRIB requires eccodes, netCDF requires netCDF4)"""

import json

_GRIB = (b"GRIB",)
_NETCDF = (b"CDF\x01", b"CDF\x02", b"CDF\x05", b"\x89HDF")


def _grib(path) -> dict:
    try:
        import eccodes
    except ImportError as e:
        raise ImportError("gridspec_from_file: GRIB requires the eccodes module (pip install mir-python[grib])") from e

    specs = {}
    with open(path, "rb") as f:
        n = 0
        while (h := eccodes.codes_grib_new_from_file(f)) is not None:
            n += 1
            try:
                name = eccodes.codes_get_string(h, "shortName")
                try:
                    spec = eccodes.codes_get_string(h, "gridSpec")
                except eccodes.CodesInternalError as e:
                    raise RuntimeError(
                        "gridspec_from_file: GRIB key 'gridSpec' requires eccodes with eckit::geo (ECCODES_ECKIT_GEO)"
                    ) from e
                specs[f"{n}:{name}"] = json.loads(spec)
            finally:
                eccodes.codes_release(h)
    return specs


def _netcdf(path) -> dict:
    from mir.netcdf import grids

    return grids(path)


def gridspec_from_file(path) -> dict:
    """Grid of each field, by '<message number>:<shortName>' (GRIB) or variable name (netCDF)"""
    with open(path, "rb") as f:
        magic = f.read(4)

    if magic in _GRIB:
        return _grib(path)
    if magic in _NETCDF:
        return _netcdf(path)
    raise ValueError(f"gridspec_from_file: '{path}' is neither GRIB nor netCDF")
